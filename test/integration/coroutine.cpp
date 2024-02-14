#include <boost/test/unit_test.hpp>

#include <boost/asio/use_awaitable.hpp>
#ifdef BOOST_ASIO_HAS_CO_AWAIT

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/ssl.hpp>

#include <boost/beast/websocket.hpp>

#include <async_mqtt5.hpp>

namespace boost::beast::websocket {

template <typename TeardownHandler>
void async_teardown(
	boost::beast::role_type /* role */,
	asio::ssl::stream<asio::ip::tcp::socket>& stream,
	TeardownHandler&& handler
) {
	return stream.async_shutdown(std::forward<TeardownHandler>(handler));
}

} // end namespace boost::beast::websocket

namespace async_mqtt5 {

template <typename StreamBase>
struct tls_handshake_type<asio::ssl::stream<StreamBase>> {
	static constexpr auto client = asio::ssl::stream_base::client;
	static constexpr auto server = asio::ssl::stream_base::server;
};

template <typename StreamBase>
void assign_tls_sni(
	const authority_path& ap,
	asio::ssl::context& /* ctx */,
	asio::ssl::stream<StreamBase>& stream
) {
	SSL_set_tlsext_host_name(stream.native_handle(), ap.host.c_str());
}

} // end namespace async_mqtt5

BOOST_AUTO_TEST_SUITE(coroutine/*, *boost::unit_test::disabled()*/)

using namespace async_mqtt5;
namespace asio = boost::asio;

constexpr auto use_nothrow_awaitable = asio::as_tuple(asio::use_awaitable);

template<typename StreamType, typename TlsContext>
asio::awaitable<void> sanity_check(mqtt_client<StreamType, TlsContext>& c) {
	auto [ec_0] = co_await c.template async_publish<qos_e::at_most_once>(
		"test/mqtt-test", "hello world with qos0!", retain_e::yes, publish_props {},
		use_nothrow_awaitable
	);
	BOOST_TEST_WARN(!ec_0);

	auto [ec_1, puback_rc, puback_props] = co_await c.template async_publish<qos_e::at_least_once>(
		"test/mqtt-test", "hello world with qos1!",
		retain_e::yes, publish_props {},
		use_nothrow_awaitable
	);
	BOOST_TEST_WARN(!ec_1);
	BOOST_TEST_WARN(!puback_rc);

	auto [ec_2, pubcomp_rc, pubcomp_props] = co_await c.template async_publish<qos_e::exactly_once>(
		"test/mqtt-test", "hello world with qos2!",
		retain_e::yes, publish_props {},
		use_nothrow_awaitable
	);
	BOOST_TEST_WARN(!ec_2);
	BOOST_TEST_WARN(!pubcomp_rc);

	subscribe_topic sub_topic = subscribe_topic {
		"test/mqtt-test", async_mqtt5::subscribe_options {
			qos_e::at_least_once,
			no_local_e::no,
			retain_as_published_e::retain,
			retain_handling_e::send
		}
	};
	
	auto [sub_ec, sub_codes, sub_props] = co_await c.async_subscribe(
		sub_topic, subscribe_props {}, use_nothrow_awaitable
	);
	BOOST_TEST_WARN(!sub_ec);
	if (!sub_codes[0])
		auto [rec, topic, payload, publish_props] = co_await c.async_receive(use_nothrow_awaitable);

	auto [unsub_ec, unsub_codes, unsub_props] = co_await c.async_unsubscribe(
		"test/mqtt-test", unsubscribe_props {},
		use_nothrow_awaitable
	);
	BOOST_TEST_WARN(!unsub_ec);
	BOOST_TEST_WARN(!unsub_codes[0]);

	co_await c.async_disconnect(use_nothrow_awaitable);
	co_return;
}


BOOST_AUTO_TEST_CASE(tcp_client_check) {
	asio::io_context ioc;

	using stream_type = asio::ip::tcp::socket;
	using client_type = mqtt_client<stream_type>;
	client_type c(ioc);

	c.brokers("broker.hivemq.com", 1883)
		.will({ "test/mqtt-test", "Client disconnected!", qos_e::at_least_once })
		.async_run(asio::detached);

	asio::steady_timer timer(ioc);
	timer.expires_after(std::chrono::seconds(5));

	timer.async_wait(
		[&](boost::system::error_code ec) {
			BOOST_TEST_WARN(ec, "Failed to receive all the expected replies!");
			c.cancel();
		}
	);

	co_spawn(ioc, 
		[&]() -> asio::awaitable<void> {
			co_await sanity_check(c);
			timer.cancel();
		},
		asio::detached
	);

	ioc.run();
}

BOOST_AUTO_TEST_CASE(websocket_tcp_client_check) {
	asio::io_context ioc;

	using stream_type = boost::beast::websocket::stream<
		asio::ip::tcp::socket
	>;

	using client_type = mqtt_client<stream_type>;
	client_type c(ioc);

	c.brokers("broker.hivemq.com/mqtt", 8000)
		.will({ "test/mqtt-test", "Client disconnected!", qos_e::at_least_once })
		.async_run(asio::detached);

	asio::steady_timer timer(ioc);
	timer.expires_after(std::chrono::seconds(5));

	timer.async_wait(
		[&](boost::system::error_code ec) {
			BOOST_TEST_WARN(ec, "Failed to receive all the expected replies!");
			c.cancel();
		}
	);

	co_spawn(ioc,
		[&]() -> asio::awaitable<void> {
			co_await sanity_check(c);
			timer.cancel();
		},
		asio::detached
	);

	ioc.run();
}


BOOST_AUTO_TEST_CASE(openssl_tls_client_check) {
	asio::io_context ioc;

	using stream_type = asio::ssl::stream<asio::ip::tcp::socket>;
	asio::ssl::context tls_context(asio::ssl::context::tls_client);

	using client_type = mqtt_client<stream_type, decltype(tls_context)>;
	client_type c(ioc, std::move(tls_context));

	c.brokers("broker.hivemq.com", 8883)
		.will({ "test/mqtt-test", "Client disconnected!", qos_e::at_least_once })
		.async_run(asio::detached);

	asio::steady_timer timer(ioc);
	timer.expires_after(std::chrono::seconds(5));

	timer.async_wait(
		[&](boost::system::error_code ec) {
			BOOST_TEST_WARN(ec, "Failed to receive all the expected replies!");
			c.cancel();
		}
	);

	co_spawn(ioc,
		 [&]() -> asio::awaitable<void> {
			 co_await sanity_check(c);
			 timer.cancel();
		 },
		 asio::detached
	);

	ioc.run();
}

BOOST_AUTO_TEST_CASE(websocket_tls_client_check) {
	asio::io_context ioc;

	using stream_type = boost::beast::websocket::stream<
		asio::ssl::stream<asio::ip::tcp::socket>
	>;

	asio::ssl::context tls_context(asio::ssl::context::tls_client);

	using client_type = mqtt_client<stream_type, decltype(tls_context)>;
	client_type c(ioc, std::move(tls_context));

	c.brokers("broker.hivemq.com/mqtt", 8884)
		.will({ "test/mqtt-test", "Client disconnected!", qos_e::at_least_once })
		.async_run(asio::detached);

	asio::steady_timer timer(ioc);
	timer.expires_after(std::chrono::seconds(5));

	timer.async_wait(
		[&](boost::system::error_code ec) {
			BOOST_TEST_WARN(ec, "Failed to receive all the expected replies!");
			c.cancel();
		}
	);

	co_spawn(ioc,
		[&]() -> asio::awaitable<void> {
			co_await sanity_check(c);
			timer.cancel();
		},
		asio::detached
	);

	ioc.run();
}

BOOST_AUTO_TEST_SUITE_END()

#endif
