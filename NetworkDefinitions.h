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

#ifndef _NETWORKDEFINITIONS_H_
#define _NETWORKDEFINITIONS_H_

#include "ios_fastmessage.h"
#include "ios_igpmessagebox.h"
#include "ios_udpmessagebox.h"

typedef ios_fc::UDPMessageBox<ios_fc::FastMessage> FPLANMessageBox;

typedef ios_fc::UDPMessageBox<ios_fc::FastMessage> FPInternetP2PMessageBox;

typedef ios_fc::IgpMessageBox FPServerMessageBox;

#endif // _NETWORKDEFINITIONS_H_
