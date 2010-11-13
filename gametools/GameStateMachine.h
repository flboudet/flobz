/* FloboPuyo
 * Copyright (C) 2004
 *   Florent Boudet        <flobo@ios-software.com>,
 *   Jean-Christophe Hoelt <jeko@ios-software.com>,
 *   Guillaume Borios      <gyom@ios-software.com>
 *
 * iOS Software <http://www.ios-software.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *
 */

#ifndef _GAMESTATEMACHINE_H_
#define _GAMESTATEMACHINE_H_

#include <stdlib.h>

class GameStateMachine;

/**
 * A generic state for the generic state machine
 */
class GameState {
public:
    GameState() : m_owner(NULL) {}
    virtual ~GameState() {}
    /// Hooks on entering and exiting the state
    virtual void enterState() {}
    virtual void exitState()  {}
    /// Returns true if the current state is completed
    virtual bool evaluate() = 0;
    /// Returns the next state of the state machine
    virtual GameState *getNextState() = 0;
    /// Asks for reevaluation of the state machine
    void evaluateStateMachine();
private:
    GameStateMachine *m_owner;
    friend class GameStateMachine;
};

/**
 * A generic state machine
 */
class GameStateMachine
{
public:
    GameStateMachine();
    GameStateMachine(GameState *initialState);
    void setInitialState(GameState *initialState);
    void evaluate();
    void reset();
private:
    GameState *m_initialState;
    GameState *m_currentState;
    bool m_currentStateActive;
};

#endif // _GAMESTATEMACHINE_H_

