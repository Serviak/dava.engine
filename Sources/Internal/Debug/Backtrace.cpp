/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "Debug/Backtrace.h"
#include "FileSystem/Logger.h"
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
#include <execinfo.h>
#include <cxxabi.h>
#include <stdlib.h>
#elif defined(__DAVAENGINE_WIN32__)
#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")
#endif


namespace DAVA 
{
//namespace Backtrace 
//{
class BacktraceTree
{
public:
    class BacktraceTreeNode
    {
    public:
        BacktraceTreeNode(void * _pointer, BacktraceTreeNode *parent)
        {
            pointer = _pointer;
        }
        ~BacktraceTreeNode()
        {
            for (uint32 k = 0; k < children.size(); ++k)
            {
                delete children[k];
                children[k] = 0;
            }
            children.clear();
        }
        
        void Insert(void * ptr)
        {
            BacktraceTreeNode * newNode = new BacktraceTreeNode(ptr, this);
            children.push_back(newNode);
            std::sort(children.begin(), children.end());
        }
        BacktraceTreeNode * parent;
        void * pointer;
        Vector<BacktraceTreeNode*> children;
    };
    
    BacktraceTreeNode * head;
    
    BacktraceTree()
    {
        head = new BacktraceTreeNode(0, 0);
    }
    
    ~BacktraceTree()
    {
        delete head;
        head = 0;
    }
    
    void Insert(Backtrace * backtrace)
    {
        Insert(head, backtrace, backtrace->size - 1);
    }
    
    void Insert(BacktraceTreeNode * head, Backtrace * backtrace, uint32 depth)
    {
        uint32 size = (uint32)head->children.size();
        for (uint32 k = 0; k < size; ++k)
        {
            if (head->children[k]->pointer == backtrace->array[depth])
            {
                Insert(head->children[k], backtrace, depth - 1);
            }
        }
        
        head->Insert(backtrace->array[depth]);
        Insert(head->children[(uint32)head->children.size() - 1], backtrace, depth - 1);
    }
        
    Backtrace* GetBacktraceByTreeNode(BacktraceTreeNode * node)
    {
        Backtrace * backtrace = (Backtrace*) malloc(sizeof(Backtrace));
        
        backtrace->size = 0;
        while(1)
        {
            backtrace->array[backtrace->size] = node->pointer;
            backtrace->size++;
            node = node->parent;
            if (node->pointer == 0)break;
        }
        return 0;
    }
};

    
    
    
    Backtrace * CreateBacktrace()
    {
        Backtrace * bt = (Backtrace*) malloc(sizeof(Backtrace));
        //GetBacktrace(bt);
        return bt;
    }
    
    void GetBacktrace(Backtrace * bt)
    {
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
        bt->size = backtrace(bt->array, MAX_BACKTRACE_DEPTH);
#elif defined(__DAVAENGINE_WIN32__)
        bt->size = CaptureStackBackTrace( 0, MAX_BACKTRACE_DEPTH, bt->array, NULL);
#endif
        pointer_size hash = 0;
        for (uint32 k = 0; k < bt->size; ++k)
        {
            hash += (k + 1) * (pointer_size)bt->array[k];
        }
        bt->hash = hash;
    }
    
    void ReleaseBacktrace(Backtrace * backtrace)
    {
        free(backtrace);
    }
    
    void CreateBacktraceLog(Backtrace * backtrace, BacktraceLog * log)
    {
        log->strings = (char**)malloc(sizeof(char*) * backtrace->size); 
        log->size = backtrace->size;
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
        char **strings = backtrace_symbols(backtrace->array, backtrace->size); 
        
        // allocate string which will be filled with the demangled function name
        size_t funcnamesize = 0;
        
        // iterate over the returned symbol lines. skip the first, it is the
        // address of this function.
        for (uint32 i = 0; i < backtrace->size; i++)
        {
            log->strings[i] = (char*)malloc(512);
            
            int len = strlen(strings[i]);
            char * temp = (char*)malloc(sizeof(char) * len + 100);
            strcpy(temp, strings[i]);

            char * tokens[100];
            int32 tokenCount = 0;
            tokens[tokenCount++] = strtok(temp," \t");
            while (tokens[tokenCount - 1] != NULL)
            {
                //Logger::FrameworkDebug("%s\n",tokens[tokenCount]);
                tokens[tokenCount++] = strtok (NULL, " \t");
                if (tokenCount > 5)break;
            }
            
            if (tokenCount >= 4)
            {
                int status = -2;
                char* ret = abi::__cxa_demangle(tokens[3],
                                                NULL, &funcnamesize, &status);
                if (status == 0) 
                {
                    //funcname = ret; // use possibly realloc()-ed string
                    
                    snprintf(log->strings[i], 512, "%s(%s)",
                            strings[i], ret);
                }
                else {
                    snprintf(log->strings[i], 512, "%s(%s)", strings[i], tokens[3]);
                }
                
                free(ret);
            }else
            {
                snprintf(log->strings[i], 512, "%s", strings[i]);
            }
            free(temp);
        }
        free(strings);
#elif defined(__DAVAENGINE_WIN32__)
        
        SYMBOL_INFO  * symbol;
        HANDLE         process;
        
        process = GetCurrentProcess();
        
        SymInitialize( process, NULL, TRUE );
        
        symbol               = ( SYMBOL_INFO * )calloc( sizeof( SYMBOL_INFO ) + 256 * sizeof( char ), 1 );
        symbol->MaxNameLen   = 255;
        symbol->SizeOfStruct = sizeof( SYMBOL_INFO );
        
        for(uint32 i = 0; i < backtrace->size; i++ )
        {
            SymFromAddr( process, (DWORD64)( backtrace->array[i] ), 0, symbol );
            log->strings[i] = (char*)malloc(512);
			_snprintf(log->strings[i], 512, "%i: %s - 0x%0X\n", backtrace->size - i - 1, symbol->Name, symbol->Address);
            //printf( "%i: %s - 0x%0X\n", frames - i - 1, symbol->Name, symbol->Address );
        }
        
        free( symbol );
#endif
    }
    
    void ReleaseBacktraceLog(BacktraceLog * log)
    {
        for (uint32 k = 0; k < log->size; ++k)
            free(log->strings[k]);
        free(log->strings);
    }
    
    void PrintBackTraceToLog()
    {
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
#define BACKTRACE_SIZ 100
        void    *array[BACKTRACE_SIZ];
        size_t  size, i;
        char    **strings;
        
        size = backtrace(array, BACKTRACE_SIZ);
        strings = backtrace_symbols(array, size);
        
        for (i = 0; i < size; ++i) {
            Logger::FrameworkDebug("%p : %s\n", array[i], strings[i]);
        }
        free(strings);
#elif defined(__DAVAENGINE_WIN32__)
    // Check out this function http://msdn.microsoft.com/en-us/library/windows/desktop/bb204633(v=vs.85).aspx
    /* Should be able to do the same stuff on windows. 
         USHORT WINAPI CaptureStackBackTrace(
         __in       ULONG FramesToSkip,
         __in       ULONG FramesToCapture,
         __out      PVOID *BackTrace,
         __out_opt  PULONG BackTraceHash
         );        
     */
#endif
    }
//}

};