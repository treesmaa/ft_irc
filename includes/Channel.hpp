#ifndef CHANNEL_HPP
#define CHANNEL_HPP

class Channel {
    public:
        Channel();
        ~Channel();
    private:
        Channel(const Channel& original);
        Channel& operator=(const Channel& other);
};
#endif