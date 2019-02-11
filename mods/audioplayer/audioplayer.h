#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <functional>

class audio_exception : public std::exception {
public:
    audio_exception(const std::string &msg) : msg(msg) {}
    virtual const char *what() const throw() { return msg.c_str(); }

    std::string msg;
};

class InternalPlayer;

class AudioPlayer {
public:
    AudioPlayer(int hz = 44100);
    AudioPlayer(std::function<void(int16_t *, int)> cb, int hz = 44100);

    void play(std::function<void(int16_t *, int)> cb);
    void close();

    void pause();
    void resume();

    void set_volume(int percent);

    void touch() const {}

    int get_delay();

private:
        std::shared_ptr<InternalPlayer> internalPlayer;
};

