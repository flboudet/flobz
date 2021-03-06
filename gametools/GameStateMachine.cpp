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

#include "GameStateMachine.h"

void GameState::evaluateStateMachine()
{
    if (m_owner != NULL)
        m_owner->evaluate();
}

GameStateMachine::GameStateMachine()
    : m_initialState(NULL),
      m_currentState(NULL),
      m_currentStateActive(false)
{
}
GameStateMachine::GameStateMachine(GameState *initialState)
    : m_initialState(initialState),
      m_currentState(initialState),
      m_currentStateActive(false)
{
}

void GameStateMachine::setInitialState(GameState *initialState)
{
    m_initialState = initialState;
    reset();
}

void GameStateMachine::evaluate()
{
    if (m_currentState == NULL)
        return;
    if (! m_currentStateActive) {
        m_currentState->enterState();
        m_currentStateActive = true;
    }
    if (m_currentState->evaluate()) {
        m_currentState->exitState();
        m_currentState->m_owner = NULL;
        m_currentState = m_currentState->getNextState();
        m_currentStateActive = false;
        m_currentState->m_owner = this;
        evaluate();
    }
}

void GameStateMachine::reset()
{
    m_currentState = m_initialState;
    m_currentState->m_owner = this;
    m_currentStateActive = false;
}

