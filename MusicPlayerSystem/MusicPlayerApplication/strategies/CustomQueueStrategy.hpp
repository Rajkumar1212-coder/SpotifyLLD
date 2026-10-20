
#pragma once

#include <iostream>
#include <queue>
#include <stack>
#include <stdexcept>
#include <vector>

#include "../models/Playlist.hpp"
#include "PlayStrategy.hpp"

using namespace std;

class CustomQueueStrategy : public PlayStrategy {
private:
    Playlist* currentPlaylist;
    int currentIndex;

    queue<Song*> nextQueue;
    stack<Song*> prevStack;

    // Get the next song from the playlist sequentially.
    Song* nextSequential() {
        if (!currentPlaylist) {
            throw runtime_error("No playlist loaded.");
        }

        const auto& songs = currentPlaylist->getSongs();

        if (currentIndex + 1 >= (int)songs.size()) {
            throw runtime_error("No next song available.");
        }

        currentIndex++;
        return songs[currentIndex];
    }

    // Get the previous song from the playlist sequentially.
    Song* previousSequential() {
        if (!currentPlaylist) {
            throw runtime_error("No playlist loaded.");
        }

        const auto& songs = currentPlaylist->getSongs();

        if (currentIndex - 1 < 0) {
            throw runtime_error("No previous song available.");
        }

        currentIndex--;
        return songs[currentIndex];
    }

    // Find the index of a song in the current playlist.
    int findSongIndex(Song* song) const {
        if (!currentPlaylist || !song) {
            return -1;
        }

        const auto& songs = currentPlaylist->getSongs();

        for (int i = 0; i < (int)songs.size(); i++) {
            if (songs[i] == song) {
                return i;
            }
        }

        return -1;
    }

    void clearNextQueue() {
        while (!nextQueue.empty()) {
            nextQueue.pop();
        }
    }

    void clearPreviousStack() {
        while (!prevStack.empty()) {
            prevStack.pop();
        }
    }

public:
    CustomQueueStrategy()
        : currentPlaylist(nullptr), currentIndex(-1) {}

    void setPlaylist(Playlist* playlist) override {
        currentPlaylist = playlist;
        currentIndex = -1;

        clearNextQueue();
        clearPreviousStack();
    }

    bool hasNext() override {
        if (!currentPlaylist) {
            return false;
        }

        // A queued song takes priority over sequential playback.
        if (!nextQueue.empty()) {
            return true;
        }

        return currentIndex + 1 <
               (int)currentPlaylist->getSize();
    }

    Song* next() override {
        if (!currentPlaylist) {
            throw runtime_error("No playlist loaded.");
        }

        if (currentPlaylist->getSize() == 0) {
            throw runtime_error("Playlist is empty.");
        }

        Song* song = nullptr;

        // Queued songs take priority.
        if (!nextQueue.empty()) {
            song = nextQueue.front();
            nextQueue.pop();

            int index = findSongIndex(song);

            // Only update the playlist index if the song
            // actually belongs to the current playlist.
            if (index != -1) {
                currentIndex = index;
            } else {
                // A queued song outside the playlist does not
                // have a meaningful sequential playlist index.
                currentIndex = -1;
            }
        } else {
            song = nextSequential();
        }

        // Store the song just played so previous() can return it.
        prevStack.push(song);

        return song;
    }

    bool hasPrevious() override {
        if (!currentPlaylist) {
            return false;
        }

        // History contains the song most recently played.
        // A previous song exists if there is an earlier entry.
        return prevStack.size() > 1 ||
               currentIndex > 0;
    }

    Song* previous() override {
        if (!currentPlaylist) {
            throw runtime_error("No playlist loaded.");
        }

        if (currentPlaylist->getSize() == 0) {
            throw runtime_error("Playlist is empty.");
        }

        // Do not remove the only history entry.
        if (prevStack.size() <= 1) {
            if (currentIndex <= 0) {
                throw runtime_error("No previous song available.");
            }

            // Remove the current song from history.
            prevStack.pop();

            // Move to the previous sequential song.
            Song* song = previousSequential();
            prevStack.push(song);

            return song;
        }

        // Discard the current song and return to the
        // previously played song.
        prevStack.pop();
        Song* song = prevStack.top();

        int index = findSongIndex(song);

        if (index != -1) {
            currentIndex = index;
        }

        return song;
    }

    void addToNext(Song* song) override {
        if (!song) {
            throw runtime_error("Cannot enqueue null song.");
        }

        nextQueue.push(song);
    }
};
