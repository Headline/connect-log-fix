/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod Sample Extension
 * Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#include "extension.h"
#include <extensions/ISDKTools.h>

#include <igameevents.h>
#include <iclient.h>
#include <iserver.h>
#include <inetchannel.h>

Sample g_Sample;
IServer *server;

IGameEventManager2 *eventmgr;

SMEXT_LINK(&g_Sample);

SH_DECL_HOOK2(IGameEventManager2, FireEvent, SH_NOATTRIB, 0, bool, IGameEvent *, bool);

bool Sample::SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	GET_V_IFACE_CURRENT(GetEngineFactory, eventmgr, IGameEventManager2, INTERFACEVERSION_GAMEEVENTSMANAGER2);

	SH_ADD_HOOK(IGameEventManager2, FireEvent, eventmgr, SH_MEMBER(this, &Sample::Hook_FireEvent), false);

	return true;
}

bool Sample::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
	sharesys->AddDependency(myself, "sdktools.ext", true, true);

	ISDKTools *pSDKTools;
	SM_GET_IFACE(SDKTOOLS, pSDKTools);

	if ((server = pSDKTools->GetIServer()) == nullptr)
	{
		Q_snprintf(error, maxlength, "Couldn't find IServer interface");
		return false;
	}

	return true;
}

bool Sample::SDK_OnMetamodUnload(char *error, size_t maxlength)
{
	SH_REMOVE_HOOK(IGameEventManager2, FireEvent, eventmgr, SH_MEMBER(this, &Sample::Hook_FireEvent), false);

	return true;
}

bool Sample::Hook_FireEvent(IGameEvent *event, bool bDontBroadcast)
{
	// One billion safety checks. Failures here aren't critical and we don't want to crash from bad input.
	if (event && !Q_strcmp(event->GetName(), "player_connect"))
	{
		int index = event->GetInt("index");
		if (index >= 0 && index < server->GetClientCount())
		{
			auto *pClient = server->GetClient(index);
			if (pClient)
			{
				auto *pNetChannel = server->GetClient(index)->GetNetChannel();
				event->SetString("address", pNetChannel ? pNetChannel->GetAddress() : "none");
			}
		}
	}

	RETURN_META_VALUE(MRES_IGNORED, true);
}
