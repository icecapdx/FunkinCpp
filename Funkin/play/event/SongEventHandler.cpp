#include "SongEventHandler.h"
#include "../PlayState.h"
#include "../CameraManager.h"
#include "../song/Conductor.h"
#include <algorithm>
#include <iostream>

SongEventHandler::SongEventHandler() = default;

void SongEventHandler::loadEvents(std::vector<SongEvent>& evts) {
    events = &evts;
    std::stable_sort(evts.begin(), evts.end(),
        [](const SongEvent& a, const SongEvent& b) { return a.time < b.time; });
}

void SongEventHandler::update(float songPosition) {
    if (!events) return;
    for (auto& evt : *events) {
        if (!evt.activated && evt.time <= songPosition) {
            evt.activated = true;
            dispatch(evt);
        }
    }
}

void SongEventHandler::reset() {
    if (!events) return;
    for (auto& evt : *events) evt.activated = false;
}

void SongEventHandler::dispatch(const SongEvent& e) {
    if      (e.kind == "FocusCamera")  handleFocusCamera(e);
    else if (e.kind == "ZoomCamera")   handleZoomCamera(e);
    else if (e.kind == "PlayAnimation")handlePlayAnimation(e);
    else if (e.kind == "SetCameraBop") handleSetCameraBop(e);
    else if (e.kind == "ScrollSpeed")  handleScrollSpeed(e);
    else {
        std::cout << "[Events] Unknown event: " << e.kind << std::endl;
    }
}

void SongEventHandler::handleFocusCamera(const SongEvent& e) {
    if (!PlayState::instance) return;
    CameraManager* cam = PlayState::instance->getCameraManager();
    if (!cam) return;

    int charTarget = 0;
    float xOff = 0.0f, yOff = 0.0f;

    if (e.value.is_object()) {
        if (e.value.contains("char") && e.value["char"].is_number())
            charTarget = e.value["char"].get<int>();
        if (e.value.contains("x") && e.value["x"].is_number())
            xOff = e.value["x"].get<float>();
        if (e.value.contains("y") && e.value["y"].is_number())
            yOff = e.value["y"].get<float>();
    } else if (e.value.is_number()) {
        charTarget = e.value.get<int>();
    }

    cam->focusOn(charTarget, xOff, yOff);
    std::cout << "[Events] FocusCamera char=" << charTarget << std::endl;
}

void SongEventHandler::handleZoomCamera(const SongEvent& e) {
    if (!PlayState::instance) return;
    CameraManager* cam = PlayState::instance->getCameraManager();
    if (!cam) return;

    float zoom = 1.0f;
    float durationSteps = 4.0f;
    bool direct = true;

    if (e.value.is_object()) {
        if (e.value.contains("zoom"))     zoom = e.value.value("zoom", 1.0f);
        if (e.value.contains("duration")) durationSteps = e.value.value("duration", 4.0f);
        if (e.value.contains("mode"))     direct = (e.value.value("mode", "direct") == "direct");
    } else if (e.value.is_number()) {
        zoom = e.value.get<float>();
    }

    float durationSeconds = Conductor::stepCrochet * durationSteps / 1000.0f;
    cam->tweenZoom(zoom, durationSeconds, direct);
    std::cout << "[Events] ZoomCamera zoom=" << zoom << " dur=" << durationSeconds << "s" << std::endl;
}

void SongEventHandler::handlePlayAnimation(const SongEvent& e) {
    if (!PlayState::instance) return;
    if (!e.value.is_object()) return;

    std::string target = e.value.value("target", "bf");
    std::string anim   = e.value.value("anim", "idle");
    bool force         = e.value.value("force", false);

    Character* ch = nullptr;
    if (target == "bf" || target == "boyfriend" || target == "player")
        ch = PlayState::instance->getBoyfriend();
    else if (target == "dad" || target == "opponent")
        ch = PlayState::instance->getDad();
    else if (target == "gf" || target == "girlfriend")
        ch = PlayState::instance->getGf();

    if (ch) {
        ch->playAnim(anim, force);
        std::cout << "[Events] PlayAnimation target=" << target << " anim=" << anim << std::endl;
    }
}

void SongEventHandler::handleSetCameraBop(const SongEvent& e) {
    if (!PlayState::instance) return;
    CameraManager* cam = PlayState::instance->getCameraManager();
    if (!cam) return;

    float rate      = 4.0f;
    float intensity = 1.0f;

    if (e.value.is_object()) {
        rate      = e.value.value("rate",      4.0f);
        intensity = e.value.value("intensity", 1.0f);
    }

    cam->setCameraBopRate(rate);
    cam->setCameraBopIntensity(intensity);
    std::cout << "[Events] SetCameraBop rate=" << rate << " intensity=" << intensity << std::endl;
}

void SongEventHandler::handleScrollSpeed(const SongEvent& e) {
    if (!PlayState::instance) return;

    float scroll  = 1.0f;
    bool absolute = false;

    if (e.value.is_object()) {
        scroll   = e.value.value("scroll",   1.0f);
        absolute = e.value.value("absolute", false);
    } else if (e.value.is_number()) {
        scroll = e.value.get<float>();
        absolute = true;
    }

    if (!absolute) {
        scroll *= PlayState::SONG.speed;
    }

    PlayState::SONG.speed = scroll;
    std::cout << "[Events] ScrollSpeed speed=" << scroll << std::endl;
}
