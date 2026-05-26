_This project has been created as part of the 42 curriculum by treesmaa._

# ft_irc
IRC server in C++98

## Description

## Instructions
### Connecting with Netcat

You can connect to the IRC server using `nc` (netcat) — no IRC client required.
The flag for CRLF line endings differs by OS:

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

## Resources
- [Internet Relay Chat Protocol (RFC 1459)](https://datatracker.ietf.org/doc/html/rfc1459)
- [Internet Relay Chat: Client Protocol (RFC 2812)](https://datatracker.ietf.org/doc/html/rfc2812)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
