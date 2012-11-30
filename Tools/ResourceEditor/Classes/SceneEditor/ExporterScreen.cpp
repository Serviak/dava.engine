/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Ivan "Dizz" Petrochenko
=====================================================================================*/

#include "ExporterScreen.h"

#include "CommandLineTool.h"
#include "SceneExporter.h"

#include "../Qt/Main/QtUtils.h"

void ExporterScreen::LoadResources()
{
    GetBackground()->SetColor(Color::White());
}

void ExporterScreen::UnloadResources()
{
    
}

void ExporterScreen::WillAppear()
{
    if(!SceneExporter::Instance())  new SceneExporter();
    
    errorLog.clear();
    
    Vector<String> & commandLine = Core::Instance()->GetCommandLine();
    if(CommandLineTool::Instance()->CommandIsFound(String("-export")))
    {
        
        int32 outPosition = CommandLineTool::Instance()->CommandPosition(String("-outdir"));
        int32 inPosition = CommandLineTool::Instance()->CommandPosition(String("-indir"));
        
        if(     CommandLineTool::Instance()->CheckPosition(outPosition) 
           &&   CommandLineTool::Instance()->CheckPosition(inPosition))
        {
            SceneExporter::Instance()->SetOutFolder(commandLine[outPosition + 1]);
            SceneExporter::Instance()->SetInFolder(commandLine[inPosition + 1]);

            int32 formatPosition = CommandLineTool::Instance()->CommandPosition(String("-format"));
            if(CommandLineTool::Instance()->CheckPosition(formatPosition))
            {
                SceneExporter::Instance()->SetExportingFormat(commandLine[formatPosition + 1]);
            }
            else 
            {
                SceneExporter::Instance()->SetExportingFormat(String(".png"));
            }
            
            int32 filePosition = CommandLineTool::Instance()->CommandPosition(String("-processfile"));
            int32 folderPosition = CommandLineTool::Instance()->CommandPosition(String("-processdir"));
            if(CommandLineTool::INVALID_POSITION != filePosition)
            {
                if(CommandLineTool::Instance()->CheckPosition(filePosition))
                {
                    SceneExporter::Instance()->ExportFile(commandLine[filePosition + 1], errorLog);
                    
                    //TODO: process errors
                }
            }
            else if(CommandLineTool::INVALID_POSITION != folderPosition)
            {
                if(CommandLineTool::Instance()->CheckPosition(folderPosition))
                {
                    SceneExporter::Instance()->ExportFolder(commandLine[folderPosition + 1], errorLog);

                    //TODO: process errors
                }
            }
            else 
            {
//                printf("Wrong arguments\n");
//                PrintUsage();
            }
        }
    }    
    else if(CommandLineTool::Instance()->CommandIsFound(String("-clean")))
    {
        int32 position = CommandLineTool::Instance()->CommandPosition(String("-clean"));
        if(CommandLineTool::Instance()->CheckPosition(position))
        {
            SceneExporter::Instance()->CleanFolder(commandLine[position + 1], errorLog);
        }
    }
}

void ExporterScreen::DidAppear()
{
    if(0 < errorLog.size())
    {
        printf("Errors:\n");
        Logger::Error("Errors:");
        Set<String>::const_iterator endIt = errorLog.end();
        int32 index = 0;
        for (Set<String>::const_iterator it = errorLog.begin(); it != endIt; ++it)
        {
            printf("[%d] %s\n", index, (*it).c_str());
            Logger::Error(Format("[%d] %s\n", index, (*it).c_str()));
            
            ++index;
        }
        
        ShowErrorDialog(errorLog);
    }

    bool forceMode =    CommandLineTool::Instance()->CommandIsFound(String("-force"))
                    ||  CommandLineTool::Instance()->CommandIsFound(String("-forceclose"));
    if(forceMode || 0 == errorLog.size())
    {
        Core::Instance()->Quit();
    }
}
