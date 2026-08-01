## Bonus: Bot

As a bonus, this project includes an IRC bot: an independent client binary that connects to the server the same way any user would, rather than a feature built into the server itself. From the server's point of view, the bot is indistinguishable from a real user — the same connection, registration, and message flow apply.

The bot is a nod to Marvin, the Paranoid Android from *The Hitchhiker's Guide to the Galaxy*: hugely capable, thoroughly unimpressed with the tasks it's given, and helpful anyway. Its replies lean into that voice throughout.

### Features

#### Automatic

| Trigger | Behavior |
|---------|----------|
| Invited to a channel | Joins the channel and greets everyone in it. |
| A user joins a channel the bot is in | Greets the new user. |
| A banned word is said where the bot can read it | Responds with a complaint. |
| The bot shuts down | Says goodbye in every channel it is currently in before disconnecting. |

#### Prompted

| Command | Description |
|---------|-------------|
| `!time` | Reports the current time. |
| `!joke` | Tells a random joke from the bot's joke list. |
| `!panic` | Responds with `don't`. |
| `!answer` | Responds with `42`. |
| `!rand <min:max>` | Returns a random number in the given range (0 to 999,999,999). |
| `!roll <count>d<sides>` | Rolls up to 10 dice with up to 999,999,999 sides each, and reports each result and the total. |

### Architecture

#### Overview

The bot is a single `Bot` class that connects to the server as an ordinary client over a non-blocking TCP socket. It performs the standard registration handshake (`PASS`, `NICK`, `USER`) and then reads from the server using `poll()`, following the same event-driven approach as the server itself.

#### Message Flow

1. `poll()` reports that the socket is ready for reading.
2. Incoming bytes are appended to an internal buffer.
3. Complete lines terminated with `CRLF` are extracted from the buffer.
4. Each line is tokenized and checked against server events (`PING`, `JOIN`, `INVITE`, ...) and bot prompts (`!time`, `!roll`, ...).
5. A match triggers a response, sent back to the server as a raw IRC command (`PRIVMSG`, `JOIN`, `PONG`, ...).

### Project Structure

```text
.
├── Makefile
├── README.md
├── includes
│   └── Bot.hpp
├── src
│   ├── Bot.cpp
│   └── main.cpp
└── texts
    ├── bad.txt
    └── jokes.txt
```

### Instructions

#### Requirements

Same as the server: a C++98-compatible compiler, `make`, and a Unix-like environment. The bot also needs an IRC server to connect to — this one, or any other RFC 1459-compatible server.

#### Building and Running

The bot is built by the same top-level `Makefile` as the server, from the project root:

```sh
make bonus
```

This builds the bot from its own Makefile in `bonus_bot/` and links `./marvin` next to `./ircserv` at the project root. `make clean` and `make fclean` clean up the bot's build artifacts too.

**Start the bot**

```sh
./marvin <network> <port> <password> [#channel1,#channel2,...]
```

**Example**

```sh
./marvin localhost 6667 mypassword "#general,#random"
```

* **network**: hostname or IP address of the IRC server to connect to
* **port**: port the IRC server is listening on
* **password**: the server's connection password
* **channels** *(optional)*: a comma-separated list of channels to join on startup; each name must start with `#` or `&`, and the whole list must be quoted so the shell passes it as a single argument. If a channel doesn't exist yet, the bot becomes its operator, like any client that creates a channel. If it exists and is password-protected, the bot simply skips it and keeps running.

The bot can also be added to a channel later, at any time, by inviting it — no restart required.
