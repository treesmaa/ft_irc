_This project has been created as part of the 42 curriculum by teesmaa, rjuhasz._

# ft_irc
IRC server in C++98

## Description

## Instructions

### Building and Running

**Build the server**
```sh
make
```

**Start the server**
```sh
./ircserv <port> <password>
```

**Example**
```sh
./ircserv 6667 mypassword
```

- **port**: the port number the server will listen on (e.g. `6667`)
- **password**: the connection password clients must provide to register

### Connecting with Netcat

You can connect to the IRC server using `nc` (netcat) — no IRC client required.

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

### Registration

Once connected, identify yourself to the server:

```
PASS yourpassword
NICK yournick
USER username 0 * :Your Real Name
```

### Connecting with a Real IRC Client

Instead of using nc, you can connect to your IRC server using standard IRC clients, _e.g._ irssi, WeeChat, or GUI clients like HexChat and mIRC.

#### Connection Settings

Use the following settings in any IRC client:

- **Server / Host**: localhost (or your server IP)
- **Port**: 6667 (or your configured port)
- **Password**: your server password
- **Nickname**: your IRC nickname
- **Username**: your IRC username

**Example**
```sh
irssi
```
Then inside the client:

```sh
/connect localhost 6667 yourpassword
/nick yournick
/user username 0 * Your Real Name
```

## Resources
- [Internet Relay Chat Protocol (RFC 1459)](https://datatracker.ietf.org/doc/html/rfc1459)
- [Internet Relay Chat: Client Protocol (RFC 2812)](https://datatracker.ietf.org/doc/html/rfc2812)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
