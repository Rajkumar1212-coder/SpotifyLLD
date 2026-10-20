
#pragma once

#include <iostream>
#include <vector>
#include <stack>
#include <random>
#include <algorithm>
#include <stdexcept>

#include "../models/Playlist.hpp"
#include "PlayStrategy.hpp"

using namespace std;

class RandomPlayStrategy : public PlayStrategy {
private:
    Playlist* currentPlaylist;

    // Songs that have not yet been played in this shuffle cycle.
    vector<Song*> remainingSongs;

    // Songs played in the current shuffle cycle.
    vector<Song*> history;

    // Index of the currently playing song in history.
    int currentIndex;

    mt19937 rng{random_device{}()};

public:
    RandomPlayStrategy()
        : currentPlaylist(nullptr), currentIndex(-1) {}

    void setPlaylist(Playlist* playlist) override {
        currentPlaylist = playlist;

        remainingSongs.clear();
        history.clear();
        currentIndex = -1;

        if (!currentPlaylist || currentPlaylist->getSize() == 0) {
            return;
        }

        remainingSongs = currentPlaylist->getSongs();

        // Shuffle the songs into random order.
        shuffle(remainingSongs.begin(), remainingSongs.end(), rng);
    }

    bool hasNext() override {
        return currentPlaylist != nullptr &&
               !remainingSongs.empty();
    }

    Song* next() override {
        if (!currentPlaylist) {
            throw runtime_error("No playlist loaded.");
        }

        if (currentPlaylist->getSize() == 0) {
            throw runtime_error("Playlist is empty.");
        }

        if (remainingSongs.empty()) {
            throw runtime_error("No songs left to play.");
        }

        // Select the next shuffled song.
        Song* selectedSong = remainingSongs.back();
        remainingSongs.pop_back();

        // If the user went backward, discard the old
        // forward history before starting a new path.
        if (currentIndex + 1 < (int)history.size()) {
            history.erase(
                history.begin() + currentIndex + 1,
                history.end()
            );
        }

        history.push_back(selectedSong);
        currentIndex++;

        return selectedSong;
    }

    bool hasPrevious() override {
        return currentIndex > 0;
    }

    Song* previous() override {
        if (!hasPrevious()) {
            throw runtime_error("No previous song available.");
        }

        // Return the previous song without deleting it
        // from history, so forward navigation stays possible.
        currentIndex--;

        return history[currentIndex];
    }
};
