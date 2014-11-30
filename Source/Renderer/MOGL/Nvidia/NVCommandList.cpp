/*-----------------------------------------------------------------------
  Copyright (c) 2014, NVIDIA. All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Neither the name of its contributors may be used to endorse 
     or promote products derived from this software without specific
     prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------*/

#include "NVCommandList.h"

PFNGLCREATESTATESNVPROC __glewCreateStatesNV;
PFNGLDELETESTATESNVPROC __glewDeleteStatesNV;
PFNGLISSTATENVPROC __glewIsStateNV;
PFNGLSTATECAPTURENVPROC __glewStateCaptureNV;
PFNGLDRAWCOMMANDSNVPROC __glewDrawCommandsNV;
PFNGLDRAWCOMMANDSADDRESSNVPROC __glewDrawCommandsAddressNV;
PFNGLDRAWCOMMANDSSTATESNVPROC __glewDrawCommandsStatesNV;
PFNGLDRAWCOMMANDSSTATESADDRESSNVPROC __glewDrawCommandsStatesAddressNV;
PFNGLCREATECOMMANDLISTSNVPROC __glewCreateCommandListsNV;
PFNGLDELETECOMMANDLISTSNVPROC __glewDeleteCommandListsNV;
PFNGLISCOMMANDLISTNVPROC __glewIsCommandListNV;
PFNGLLISTDRAWCOMMANDSSTATESCLIENTNVPROC __glewListDrawCommandsStatesClientNV;
PFNGLCOMMANDLISTSEGMENTSNVPROC __glewCommandListSegmentsNV;
PFNGLCOMPILECOMMANDLISTNVPROC __glewCompileCommandListNV;
PFNGLCALLCOMMANDLISTNVPROC __glewCallCommandListNV;
PFNGLGETCOMMANDHEADERNVPROC __glewGetCommandHeaderNV;
PFNGLGETSTAGEINDEXNVPROC __glewGetStageIndexNV;


static int initedNVcommandList = 0;

int init_NV_command_list(NVPROC (*fnGetProc)(const char* name))
{
  if (initedNVcommandList) return __glewCreateStatesNV != ((void*)0);

  __glewCreateStatesNV = (PFNGLCREATESTATESNVPROC)fnGetProc("glCreateStatesNV");
  __glewDeleteStatesNV = (PFNGLDELETESTATESNVPROC)fnGetProc("glDeleteStatesNV");
  __glewIsStateNV = (PFNGLISSTATENVPROC)fnGetProc("glIsStateNV");
  __glewStateCaptureNV = (PFNGLSTATECAPTURENVPROC)fnGetProc("glStateCaptureNV");
  __glewDrawCommandsNV = (PFNGLDRAWCOMMANDSNVPROC)fnGetProc("glDrawCommandsNV");
  __glewDrawCommandsAddressNV = (PFNGLDRAWCOMMANDSADDRESSNVPROC)fnGetProc("glDrawCommandsAddressNV");
  __glewDrawCommandsStatesNV = (PFNGLDRAWCOMMANDSSTATESNVPROC)fnGetProc("glDrawCommandsStatesNV");
  __glewDrawCommandsStatesAddressNV = (PFNGLDRAWCOMMANDSSTATESADDRESSNVPROC)fnGetProc("glDrawCommandsStatesAddressNV");
  __glewCreateCommandListsNV = (PFNGLCREATECOMMANDLISTSNVPROC)fnGetProc("glCreateCommandListsNV");
  __glewDeleteCommandListsNV = (PFNGLDELETECOMMANDLISTSNVPROC)fnGetProc("glDeleteCommandListsNV");
  __glewIsCommandListNV = (PFNGLISCOMMANDLISTNVPROC)fnGetProc("glIsCommandListNV");
  __glewListDrawCommandsStatesClientNV = (PFNGLLISTDRAWCOMMANDSSTATESCLIENTNVPROC)fnGetProc("glListDrawCommandsStatesClientNV");
  __glewCommandListSegmentsNV = (PFNGLCOMMANDLISTSEGMENTSNVPROC)fnGetProc("glCommandListSegmentsNV");
  __glewCompileCommandListNV = (PFNGLCOMPILECOMMANDLISTNVPROC)fnGetProc("glCompileCommandListNV");
  __glewCallCommandListNV = (PFNGLCALLCOMMANDLISTNVPROC)fnGetProc("glCallCommandListNV");
  __glewGetCommandHeaderNV = (PFNGLGETCOMMANDHEADERNVPROC)fnGetProc("glGetCommandHeaderNV");
  __glewGetStageIndexNV = (PFNGLGETSTAGEINDEXNVPROC)fnGetProc("glGetStageIndexNV");
  
  initedNVcommandList = 1;
  
  return __glewCreateStatesNV != ((void*)0);
}

