#ifndef GAMEENGINE_H
#define GAMEENGINE_H

// includes
#include "GameState.h"
#include "GameStateFactory.h"
#include "Rendering/GameWindow.h"
#include<map>
#include<string>
#include<iostream>

using namespace std;

class GameState;

class GameEngine
{
public:
    GameEngine();
    ~GameEngine();
    // state switching methods
    void SwitchState(int stateType);
    void InvokeEngine(int width, int height, int bpp, std::string caption);
    void AddState(GameState* gs);
    void RemoveState(int stateType);

    //value  passing methods
    void SetInt(int destinationState,const std::string& key, int value);
    void SetFloat(int destinationState,const std::string& key, float value);
    void SetString(int destinationState,const std::string& key, const std::string& value);
    void SetObject(int destinationState,const std::string& key, void* value);

    void Running(bool running)
    {
        m_running = running;
    }
    bool Running()const
    {
        return m_running;
    }
    GameWindow* GetGameWindow() const
    {
        return win;
    }

protected:
private:
    std::map<int,GameState*> m_states;
    GameState* m_currentState;
    GameWindow* win;
    bool m_running;

};

#endif // GAMEENGINE_H
