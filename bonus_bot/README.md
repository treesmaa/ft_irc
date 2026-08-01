## Bonus: Bot

As a bonus we implemented a bot for this server. It pretty much acts as an independet client (its an extra binary) that runs on the server

### Usage

```bash
./marvin <network> <port> <password> optinal:<#chan1,#chan2,...>
```
The basic usage will connect the bot to the server as a client and is ready to be invited. The server does not know anything about it. For it its just another client

It optionally allows as another argument a comma seperated list of channels the bot will join. If they don't exist the bot automatically becomes moderator. If they do and they are password protected the bot will just not connect to them but stay alive

### Automatic Features

#### Auto join on invite

Inviting the bot will lead to him joining the channel and greet everyone around

#### Auto greet new users

Whenever a new user joins a channel in which the bot can read, the bot will great the new user with a message

#### Moderate Bad Words

The bot takes a list of bad words that are considered bad words. Saying these where the bot can read it will lead to an angry respose or if possible to a kick.

### Prompted Features

#### Random number

Prompt: `!rand <min:max>`

The user can request a random number in a range between 0 and 1000000000.

#### Time

Prompt: `!time`

The user can request the current time

#### Jokes

Prompt: `!joke`

The user can request the bot to tell a joke

#### Panic

Prompt: `!panic`

The bot will remember the user not to panic! (responds with `don't`)

#### Answer

Prompt: `!answer`

In case the user forgets about the answer to the universe this prompt allows them to be remembered (responds with `42`)
