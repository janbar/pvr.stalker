/*
 *      Copyright (C) 2015  Jamal Edey
 *      http://www.kenshisoft.com/
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kodi; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *  http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 *
 */

#include "CWatchdog.h"

#include "client.h"
#include "SAPI.h"
#include "SData.h"

using namespace ADDON;

CWatchdog::CWatchdog(uint32_t iInterval, sc_identity_t &identity)
  : CThread(), m_iInterval(iInterval), m_identity(identity), m_data(NULL)
{
}

CWatchdog::~CWatchdog()
{
  StopThread();
}

void CWatchdog::SetData(void *data)
{
  m_data = data;
}

void *CWatchdog::Process()
{
  XBMC->Log(LOG_DEBUG, "%s: start", __FUNCTION__);
  
  while (!IsStopped()) {
    int iCurPlayType;
    int iEventActiveId;
    Json::Value parsed;
    SError ret;
    uint32_t iNow;
    uint32_t iTarget;
    
    // hardcode values for now
    iCurPlayType = 1; // tv
    iEventActiveId = 0;
    
    ret = SAPI::GetEvents(iCurPlayType, iEventActiveId, m_identity, parsed);
    if (ret == SERROR_OK) {
      // ignore the result. don't confirm events (yet)
    } else {
      XBMC->Log(LOG_ERROR, "%s: GetEvents failed", __FUNCTION__);
      
      if (ret == SERROR_AUTHORIZATION) {
        if (m_data) {
          ret = ((SData *)m_data)->ReAuthenticate(true);
        } else {
          XBMC->Log(LOG_NOTICE, "%s: data not set. unable to request re-authentication", __FUNCTION__);
        }
      }
    }
    
    // leverage watchdog to periodically unload epg data from memory
    if (m_data)
      ((SData *)m_data)->UnloadEPG();
    
    iNow = 0;
    iTarget = m_iInterval * 1000;

    while (iNow < iTarget) {
      if (Sleep(100))
        break;
      iNow += 100;
    }
  }
  
  XBMC->Log(LOG_DEBUG, "%s: stop", __FUNCTION__);
  
  return NULL;
}
