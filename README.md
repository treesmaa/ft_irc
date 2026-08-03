_This project has been created as part of the 42 curriculum by teesmaa, rjuhasz, adeters._

# ft_irc
A custom IRC server implemented based on the IRC protocol and written in C++, which allows users to connect and send direct messages privately and on channels.

## Description
IRC (Internet Relay Chat) is a text-based chat protocol that allows for real-time online communication. An IRC server manages connections and an IRC client is a program that a user runs to connect to the server. 

ft_irc implements the core functionality of an IRC server, following the IRC protocol. Communication relies on TCP/IP and multiple clients can connect at once. The server uses the standard IRC message format (`COMMAND [parameters] [:trailing parameter]`) and numeric replies. The aim was not to implement every historical IRC feature, but to create a simple usable IRC server that is compatible with real IRC clients like irssi, WeeChat, HexChat, etc. The server has been thoroughly tested with irssi, which we chose as a reference client.

Users can connect, be authenticated, choose a nickname and username, send private messages, join and leave channels, send messages in channels, and quit. Before they can start interacting with others on the server, users register using the standard IRC handshake (which requires the client to call the `PASS`, `NICK` and `USER` commands). Channels have operators and regular users. Operators can change the modes of the channel and give/take operator privileges. In order to increase compatibility with irssi, common commands automatically sent out by the client (`CAP LS`, `PING`, `WHO`, `WHOIS` and `MODE`` queries) have been implemented.

ft_irc implements communication between IRC clients and a server. It does not implement server-to-server communication (IRC networks). Users can connect to one server, but server linking is not possible.

### Supported Commands

- **Registration & Client Support:** `PASS`, `NICK`, `USER`, `CAP`, `PING`, `WHO`, `WHOIS`
- **Messaging:** `PRIVMSG`, `NOTICE`
- **Channels:** `JOIN`, `PART`, `TOPIC`, `KICK`, `INVITE`
- **Modes:** `MODE`
  - `i` – invite-only channel
  - `t` – only channel operators may change the topic
  - `k` – set or remove a channel key (password)
  - `o` – grant or revoke channel operator privileges
  - `l` – set or remove the channel user limit
- **Disconnection:** `QUIT`

### Architecture

#### Overview

The server follows an event-driven architecture based on an event loop. It uses non-blocking sockets together with `poll()` to monitor multiple clients simultaneously without creating one thread per client.

#### Components
| Component | Responsibility |
|-----------|----------------|
| **Server** | Creates the listening socket, manages the event loop, handles communication with clients, accepts new connections, disconnects clients when needed, and stores connected clients and channels. |
| **Client** | Stores per-user state such as socket descriptor, nickname, username, registration status, receive buffer, and registered channels. |
| **Channel** | Represents an IRC channel, including its members, operators, topic, invited users, and channel modes (`+i`, `+t`, `+k`, `+l`, `+o`). |
| **Command Parser** | Splits incoming IRC messages into commands and parameters according to the IRC message format. Performs initial checks on the command format. |
| **Command Handler** | Validates commands, checks permissions, updates server, client and channel data, generates IRC replies, and requests server to send messages to clients. |

#### Message Flow

1. `poll()` notifies the server that a socket is ready for reading.
2. The server reads incoming data into the client's buffer.
3. Complete messages ending with `CRLF` are extracted.
4. The parser splits each message into a command and its parameters.
5. The command handler validates the command, performs the requested operations and generates any necessary numeric replies.
6. The server sends numeric replies, private messages, and broadcasts events or messages to the affected clients.

## Project Structure

```text
.
├── Makefile
├── README.md
├── includes
│   ├── Channel.hpp
│   ├── Client.hpp
│   ├── CommandHandler.hpp
│   ├── CommandParser.hpp
│   ├── Debug.hpp
│   ├── Server.hpp
│   └── Signal.hpp
└── src
    ├── channel
    ├── client
    ├── command
    ├── debug
    ├── server
    ├── signal
    └── main.cpp
```

## Instructions

### Requirements

- A C++98-compatible compiler
- `make`
- A Unix-like environment (Linux/macOS recommended)
- Optional: an IRC client such as `irssi` for testing

### Building and Running

**Build the server**

```sh
make
```

To build with debug flags:

```sh
make debug
```

**Start the server**

```sh
./ircserv <port> <password>
```

**Example**

```sh
./ircserv 6667 mypassword
```

* **port**: the port number the server will listen on (e.g. `6667`)
* **password**: the connection password clients must provide during registration

---

### Connecting

It is possible to connect to the server using either `nc` (netcat), a command-line TCP client, or a real IRC client.

When using netcat, users must enter IRC messages manually using the IRC message format `COMMAND [parameters] [:trailing parameter]` and perform the registration handshake themselves.

Real IRC clients provide a user interface on top of the protocol. Commands are typically entered with a preceding `/` (for example, `/join #channel`), and the client can handle parts of the registration process automatically when configured.

---

### Netcat

IRC messages must be terminated with `CRLF` (`\r\n`). Depending on the netcat version, the `-C` or `-c`` option may be required to send the correct line endings.

**Linux**

```sh
nc -C <host> <port>
```

**macOS**

```sh
nc -c <host> <port>
```

**Example**

```sh
nc -C localhost 6667   # Linux
nc -c localhost 6667   # macOS
```

#### Registration

Once connected, register with the server:

```
PASS yourpassword
NICK yournick
USER username 0 * :Your Real Name
```
---

### Connecting with a Real IRC Client

You can also connect to the server using an IRC client such as irssi, WeeChat, HexChat, or mIRC.

#### Connection Settings

Configure the client with:

* **Server / Host**: `localhost` (or your server IP)
* **Port**: `6667` (or your configured port)
* **Password**: your server password
* **Nickname**: your IRC nickname
* **Username**: your IRC username

For example, using irssi:

```sh
irssi
```

Then connect:

```
/connect localhost 6667 yourpassword
```

---

### Basic Usage

After connecting, users can interact using common IRC commands.

#### Channels

Join a channel:

```
/join #channel
```

Send a message:

```
/msg nickname Hello!
```

Send a message to a channel:

```
Hello everyone!
```

Change a channel topic:

```
/topic New topic
```

Leave a channel:

```
/part
```

Disconnect:

```
/quit
```

Channel operators can manage channels using mode commands:

```
/mode #channel +i
/mode #channel +o nickname
/mode #channel +k password
```

When using netcat, the same commands are sent without the `/` prefix and in capital letters:

```
PRIVMSG nickname :Hello!
JOIN #channel
PRIVMSG #channel :Hello everyone!
TOPIC #channel :New topic
PART #channel
QUIT
```


## Bonus: Bot

As a bonus, this project includes an IRC bot: an independent client binary that connects to the server the same way any user would. For what it does and how to build and run it, see [`bonus_bot/README.md`](bonus_bot/README.md).

## Resources

- [Internet Relay Chat Protocol (RFC 1459)](https://datatracker.ietf.org/doc/html/rfc1459)
- [Internet Relay Chat: Client Protocol (RFC 2812)](https://datatracker.ietf.org/doc/html/rfc2812)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)

### AI Usage

AI was used as a complementary learning tool alongside the listed resources. It was used to better understand networking concepts in C++, the IRC protocol, and design decisions related to implementing an event-driven server.

AI was also used as a writing and documentation aid for structuring this README, improving clarity, checking grammar and spelling, and refining technical explanations. 

All implementation decisions and code were developed and reviewed by the project authors.
