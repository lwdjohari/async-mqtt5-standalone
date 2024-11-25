Async.MQTT5: A C++17 MQTT client [Experimental]
===============================

> [!WARNING]
> Please use upstream!<br/>
> This one is experimental to add implementations for standalone ASIO, abseil & BoringSSL.<br/>
> Status : WIP, Experimental & Unstable.


Async.MQTT5 is a professional, industrial-grade C++17 client built on [Asio]([https://think-async.com/Asio/index.html]). This Client is designed for publishing or receiving messages from an MQTT 5.0 compatible Broker. Async.MQTT5 represents a comprehensive implementation of the MQTT 5.0 protocol standard, offering full support for publishing or receiving messages with QoS 0, 1, and 2. 


Motivation
---------
 The [MQTT](https://mqtt.org/) protocol is widely used for communication in various real-world scenarios, primarily as a reliable communication protocol for data transfer to and from IoT devices. While the MQTT protocol is relatively straightforward, integrating it into an application can be complex, primarily due to the challenging implementation of message retransmission after a disconnect/reconnect sequence. To address these challenges and simplify MQTT integration once and for all, Async.MQTT5 was designed with the following core objectives in mind:

 Objective | Description |
 ----------|---------|
"Plug and play" interface that abstracts away MQTT protocol internals, low-level network events, message retransmission and other complexities | Enables developers to publish or receive messages with just a single line of code, significantly reducing the learning curve and development time. Getting started requires basic MQTT knowledge, making the library accessible to developers of all skill levels. |
Highly reliable and robust Client that handles network losses, unreliable data transport, network latencies, and other unpredictable events | The Client does not expose connect functions (nor asynchronous connect functions); instead, network connectivity, MQTT handshake, and message retransmission are automatically handled within the Client while strictly adhering to the MQTT specification.  This automation eliminates the need for developers to write extensive and error-prone code to handle these scenarios, allowing them to focus on the application's core functionality. |
Complete adherence to the ~~Boost~~.Asio's universal asynchronous model | The interfaces and implementation strategies are built upon the foundations of ~~Boost~~.Asio. This compatibility enables seamless integration with any other library within the ~~Boost~~.Asio ecosystem. |

When to Use
---------
 Async.MQTT5 might be suitable for you if any of the following statements is true:

- Your application uses ~~Boost~~.Asio standalone ASIO and requires integrating an MQTT Client.
- You require asynchronous access to an MQTT Broker.
- You are developing a higher-level component that requires a connection to an MQTT Broker.
- You require a dependable and resilient MQTT Client to manage all network-related issues automatically.

It may not be suitable for you if:
- You solely require synchronous access to an MQTT Broker.
- The MQTT Broker you connect to does not support the MQTT 5 version.

Features
---------
Async.MQTT5 is a library designed with the core belief that users should focus solely on their application logic, not the network complexities.
The library attempts to embody this belief with a range of key features designed to elevate the development experience: 

Feature | Description |
--------|-------------|
**Complete TCP, TLS/SSL, ~~and WebSocket support~~** | The MQTT protocol requires an underlying network protocol that provides ordered, lossless, bi-directional connection (stream). Users can customize this stream through a template parameter. The Async.MQTT5 library has been tested with the most used transport protocols: TCP/IP using `asio::ip::tcp::socket`, TLS/SSL using `asio::ssl::stream`, ~~WebSocket/TCP and WebSocket/TLS using `boost::beast::websocket::stream`)~~ - Because currently this fork is intended for standalone ASIO and not use Boost, WS feature is not used. Please use upstream from mireo. |
**Automatic reconnect and retry mechanism** | Automatically handles connection loss, backoffs, reconnections, and message transmissions. Automating these processes enables users to focus entirely on the application's functionality. See [Built-in Auto-Reconnect and Retry Mechanism](https://spacetime.mireo.com/async-mqtt5/async_mqtt5/auto_reconnect.html). |
**Prioritised efficiency** | Utilises network and memory resources as efficiently as possible. |
**Minimal memory footprint** | Ensures optimal performance in resource-constrained environments typical of IoT devices. |
 **Completion token** | All asynchronous functions are implemented using Boost.Asio's universal asynchronous model and support CompletionToken. This allows versatile usage with callbacks, coroutines, and futures. |
**Custom allocators** | Support for custom allocators allows extra flexibility and control over the memory resources. Async.MQTT5 will use allocators associated with handlers from asynchronous functions to create instances of objects needed in the library implementation. |
**Per-operation cancellation** | All asynchronous operations support individual, targeted cancellation as per Asio's [Per-Operation Cancellation](). This enables all asynchronous functions to be used in [Parallel Operations](https://www.boost.org/doc/libs/1_86_0/doc/html/boost_asio/overview/composition/parallel_group.html), which is especially beneficial for executing operations that require a timeout mechanism.
**Full implementation of MQTT 5.0 specification** | Ensures full compatibility with [MQTT 5.0 standard](https://docs.oasis-open.org/mqtt/mqtt/v5.0/mqtt-v5.0.html). This latest version introduces essential features that enhance system robustness, including advanced error-handling mechanisms, session and message expiry, and other improvements designed to support modern IoT use cases.  |
**Support for QoS 0, QoS 1, and QoS 2**| Fully implements all quality-of-service levels defined by the MQTT protocol to match different reliability needs in message delivery. |
**Custom authentication** | Defines an interface for your custom authenticators to perform [Enhanced Authentication](https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html#_Toc3901256). In addition to username/password authentication, this customization point enables the implementation of any authentication mechanism supported by the SASL protocol, offering flexibility for advanced security requirements. |
**High availability** | Supports listing multiple Brokers within the same cluster to which the Client can connect. In the event of a connection failure with one Broker, the Client switches to the next in the list. |
**Offline buffering** | Automatically buffers all requests made while offline, ensuring they are sent when the connection is re-established. This allows users to call any asynchronous function freely, regardless of current connection status. |

Getting Started
---------
Async.MQTT5 is a header-only library. To use Async.MQTT5 it requires the following: 
- **C++17 capable compiler**
- ~~**Boost 1.82 or later**~~  **Standalone ASIO**. We are working on to add standalone ASIO. For **Beast** support TBA.
- ~~**OpenSSL**~~ **BoringSSL**. We are working on to use BoringSSL as the SSL lib. 

Reason: For enabling to non-boost projects that are already have quite heavy deps with same functionality.


Contributing
---------
When contributing to this repository, please first discuss the change you wish to make via issue, email, or any other method with the owners of this repository before making a change.

You may merge a Pull Request once you have the sign-off from other developers, or you may request the reviewer to merge it for you.

Credits
--------- 
Forked from:

Maintained and authored by [Mireo](https://www.mireo.com).

<p align="center">
<a href="https://www.mireo.com"><img height="200" alt="Mireo" src="https://www.mireo.com/img/assets/mireo-logo.svg"></img></a>
</p>
