
set(  MAIN_MODULE_VALUES 
NAME_MODULE                            #
MODULE_TYPE                            #"[ INLINE STATIC DYNAMIC ]"
#
SOURCE_FOLDERS             
ERASE_FOLDERS              
ERASE_FOLDERS_${DAVA_PLATFORM_CURENT}   
#
CPP_FILES                  
HPP_FILES                  
CPP_FILES_${DAVA_PLATFORM_CURENT}       
HPP_FILES_${DAVA_PLATFORM_CURENT}       
#
CPP_FILES_RECURSE            
HPP_FILES_RECURSE            
CPP_FILES_RECURSE_${DAVA_PLATFORM_CURENT} 
HPP_FILES_RECURSE_${DAVA_PLATFORM_CURENT} 
#
ERASE_FILES                
ERASE_FILES_${DAVA_PLATFORM_CURENT}     
ERASE_FILES_NOT_${DAVA_PLATFORM_CURENT} 
#
UNITY_IGNORE_LIST             
UNITY_IGNORE_LIST_${DAVA_PLATFORM_CURENT}  
#
INCLUDES         
INCLUDES_PRIVATE 
INCLUDES_${DAVA_PLATFORM_CURENT} 
INCLUDES_PRIVATE_${DAVA_PLATFORM_CURENT} 
#
DEFINITIONS                
DEFINITIONS_PRIVATE             
DEFINITIONS_${DAVA_PLATFORM_CURENT}     
DEFINITIONS_PRIVATE_${DAVA_PLATFORM_CURENT}  
#
STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}           
STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_RELEASE   
STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_DEBUG     
#
DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURENT}           
#
FIND_SYSTEM_LIBRARY                   
FIND_SYSTEM_LIBRARY_${DAVA_PLATFORM_CURENT}        
#
BINARY_DIR_WIN
BINARY_WIN32_DIR_RELEASE
BINARY_WIN32_DIR_DEBUG
BINARY_WIN32_DIR_RELWITHDEB
)
#
macro( setup_main_module )

    if( NOT MODULE_TYPE )
        set( MODULE_TYPE INLINE )
    endif()

    if( NOT ( ${MODULE_TYPE} STREQUAL "INLINE" ) )
        project ( ${NAME_MODULE} )
        include ( CMake-common )
    endif()

    set( INIT )

    get_filename_component (DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    get_property( DAVA_COMPONENTS GLOBAL PROPERTY  DAVA_COMPONENTS )

    list (FIND DAVA_COMPONENTS "ALL" _index)
    if ( ${_index} GREATER -1)
        set( INIT true )
    else()
        if( NAME_MODULE )
            list (FIND DAVA_COMPONENTS ${NAME_MODULE} _index)
            if ( ${_index} GREATER -1)
                set( INIT true )
            endif()
        else()
            set( INIT true )
        endif()
    endif()

    if ( INIT )
        if( IOS AND ${MODULE_TYPE} STREQUAL "DYNAMIC" )
            set( MODULE_TYPE "STATIC" )
        endif()

        #"APPLE VALUES"
        if( APPLE )
            foreach( VALUE CPP_FILES 
                           CPP_FILES_RECURSE 
                           ERASE_FILES 
                           ERASE_FILES_NOT
                           DEFINITIONS 
                           DEFINITIONS_PRIVATE 
                           INCLUDES
                           INCLUDES_PRIVATE 
                           UNITY_IGNORE_LIST )
                if( ${VALUE}_APPLE)
                    list( APPEND ${VALUE}_${DAVA_PLATFORM_CURENT} ${${VALUE}_APPLE} )  
                endif()
            endforeach()
        endif()

        if( ANDROID )
            list( APPEND STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT} ${DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURENT}}  )
        endif()
        
        if( WIN )
            list( APPEND STATIC_LIBRARIES_WIN         ${STATIC_LIBRARIES_WIN${DAVA_PROJECT_BIT}} )
            list( APPEND STATIC_LIBRARIES_WIN_RELEASE ${STATIC_LIBRARIES_WIN${DAVA_PROJECT_BIT}_RELEASE} ) 
            list( APPEND STATIC_LIBRARIES_WIN_DEBUG   ${STATIC_LIBRARIES_WIN${DAVA_PROJECT_BIT}_DEBUG} )
            list( APPEND DYNAMIC_LIBRARIES_WIN        ${DYNAMIC_LIBRARIES_WIN${DAVA_PROJECT_BIT}} )
        endif()      
       
        #"FIND LIBRARY"
        foreach( NAME ${FIND_SYSTEM_LIBRARY} ${FIND_SYSTEM_LIBRARY_${DAVA_PLATFORM_CURENT}} )
            FIND_LIBRARY( ${NAME}_LIBRARY  ${NAME} )

            if( ${NAME}_LIBRARY )
                list ( APPEND STATIC_LIBRARIES_SYSTEM_${DAVA_PLATFORM_CURENT} ${${NAME}_LIBRARY} )
            else()
                if ( NOT NOT_TARGET_EXECUTABLE )
                    find_package( ${NAME} )
                    list ( APPEND STATIC_LIBRARIES_SYSTEM_${DAVA_PLATFORM_CURENT} ${${NAME}_LIBRARY} )
                endif()
            endif()
        endforeach()        

        #"ERASE FILES"
        foreach( PLATFORM  ${DAVA_PLATFORM_LIST} )
            if( NOT ${PLATFORM} AND ERASE_FILES_NOT_${PLATFORM} )
                list( APPEND ERASE_FILES ${ERASE_FILES_NOT_${PLATFORM}} ) 
            endif()
        endforeach()
        if( ERASE_FILES_NOT_${DAVA_PLATFORM_CURENT} AND ERASE_FILES )
             list(REMOVE_ITEM ERASE_FILES ${ERASE_FILES_NOT_${DAVA_PLATFORM_CURENT}} )
        endif()

        if( SOURCE_FOLDERS )

            foreach( VALUE ${MAIN_MODULE_VALUES} )
                set( ${VALUE}_DIR_NAME ${${VALUE}} )
                set( ${VALUE}  )
            endforeach()

            define_source_folders  ( SRC_ROOT            ${SOURCE_FOLDERS_DIR_NAME}
                                     ERASE_FOLDERS       ${ERASE_FOLDERS_DIR_NAME} ${ERASE_FOLDERS_${DAVA_PLATFORM_CURENT}_DIR_NAME} )

            foreach( VALUE ${MAIN_MODULE_VALUES} )
                set(  ${VALUE} ${${VALUE}_DIR_NAME} )
            endforeach()

            set_project_files_properties( "${PROJECT_SOURCE_FILES_CPP}" )

        endif()

        #"DEFINE SOURCE"
        define_source_files (
            GLOB_CPP_PATTERNS          ${CPP_FILES}         ${CPP_FILES_${DAVA_PLATFORM_CURENT}}
            GLOB_H_PATTERNS            ${HPP_FILES}         ${HPP_FILES_${DAVA_PLATFORM_CURENT}}
            GLOB_RECURSE_CPP_PATTERNS  ${CPP_FILES_RECURSE} ${CPP_FILES_RECURSE_${DAVA_PLATFORM_CURENT}}
            GLOB_RECURSE_H_PATTERNS    ${HPP_FILES_RECURSE} ${HPP_FILES_RECURSE_${DAVA_PLATFORM_CURENT}}

            GLOB_ERASE_FILES           ${ERASE_FILES} ${ERASE_FILES_${DAVA_PLATFORM_CURENT}}
        )

        set( ALL_SRC ${CPP_FILES} ${PROJECT_SOURCE_FILES} ${H_FILES} )

        set_project_files_properties( "${ALL_SRC}" )


        #"SAVE PROPERTY"
        save_property( PROPERTY_LIST 
                DEFINITIONS
                DEFINITIONS_${DAVA_PLATFORM_CURENT}
                DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURENT}          
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT} 
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_RELEASE 
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_DEBUG 
                STATIC_LIBRARIES_SYSTEM_${DAVA_PLATFORM_CURENT}
                INCLUDES
                INCLUDES_${DAVA_PLATFORM_CURENT}
                INCLUDES_PRIVATE
                INCLUDES_PRIVATE_${DAVA_PLATFORM_CURENT}                
                BINARY_DIR_WIN
                BINARY_WIN32_DIR_RELEASE
                BINARY_WIN32_DIR_DEBUG
                BINARY_WIN32_DIR_RELWITHDEB
                )

        load_property( PROPERTY_LIST 
                DEFINITIONS
                DEFINITIONS_${DAVA_PLATFORM_CURENT}
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT} 
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_RELEASE 
                STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_DEBUG 
                STATIC_LIBRARIES_SYSTEM_${DAVA_PLATFORM_CURENT}
                INCLUDES
                INCLUDES_${DAVA_PLATFORM_CURENT}
                INCLUDES_PRIVATE
                INCLUDES_PRIVATE_${DAVA_PLATFORM_CURENT}                  
                )


        #"DEFINITIONS"
        if( DEFINITIONS )
            add_definitions( ${DEFINITIONS} )
        endif()
        if( DEFINITIONS_${DAVA_PLATFORM_CURENT} )
            add_definitions( ${DEFINITIONS_${DAVA_PLATFORM_CURENT}} )
        endif()

        #"INCLUDES_DIR"
        load_property( PROPERTY_LIST INCLUDES )
        if( INCLUDES )
            include_directories( "${INCLUDES}" )  
        endif()
        if( INCLUDES_${DAVA_PLATFORM_CURENT} )
            include_directories( "${INCLUDES_${DAVA_PLATFORM_CURENT}}" )  
        endif()

        if( ${MODULE_TYPE} STREQUAL "INLINE" )
            set (${DIR_NAME}_CPP_FILES ${CPP_FILES} PARENT_SCOPE)
            set (${DIR_NAME}_H_FILES ${H_FILES}     PARENT_SCOPE)
        else()
            project( ${NAME_MODULE} )

            generate_source_groups_project ()

            generated_unity_sources( ALL_SRC  IGNORE_LIST ${UNITY_IGNORE_LIST}
                                              IGNORE_LIST_${DAVA_PLATFORM_CURENT} ${UNITY_IGNORE_LIST_${DAVA_PLATFORM_CURENT}} ) 
                               
            if( ${MODULE_TYPE} STREQUAL "STATIC" )
                add_library( ${NAME_MODULE} STATIC  ${ALL_SRC} )
                append_property( TARGET_MODULES_LIST ${NAME_MODULE} )            
            elseif( ${MODULE_TYPE} STREQUAL "DYNAMIC" )
                add_library( ${NAME_MODULE} SHARED  ${ALL_SRC}  )
                load_property( PROPERTY_LIST TARGET_MODULES_LIST )
                append_property( TARGET_MODULES_LIST ${NAME_MODULE} )            
                add_definitions( -DDAVA_MODULE_EXPORTS )                

                if( WIN32 AND NOT DEPLOY )
                    set( BINARY_WIN32_DIR_RELEASE    "${CMAKE_CURRENT_BINARY_DIR}/Release" )
                    set( BINARY_WIN32_DIR_DEBUG      "${CMAKE_CURRENT_BINARY_DIR}/Debug" )
                    set( BINARY_WIN32_DIR_RELWITHDEB "${CMAKE_CURRENT_BINARY_DIR}/RelWithDebinfo" )
                    save_property( PROPERTY_LIST BINARY_WIN32_DIR_RELEASE 
                                                 BINARY_WIN32_DIR_DEBUG
                                                 BINARY_WIN32_DIR_RELWITHDEB )
                endif()

            endif()

            if( DEFINITIONS_PRIVATE )
                add_definitions( ${DEFINITIONS_PRIVATE} )
            endif()

            if( DEFINITIONS_PRIVATE_${DAVA_PLATFORM_CURENT} )
                add_definitions( ${DEFINITIONS_PRIVATE_${DAVA_PLATFORM_CURENT}} )
            endif()

            if( INCLUDES_PRIVATE )
                include_directories( ${INCLUDES_PRIVATE} ) 
            endif() 

            if( INCLUDES_PRIVATE_${DAVA_PLATFORM_CURENT} )
                include_directories( ${INCLUDES_PRIVATE_${DAVA_PLATFORM_CURENT}} ) 
            endif() 


            if( WIN32 )
                grab_libs(LIST_SHARED_LIBRARIES_DEBUG   "${STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_DEBUG}"   EXCLUDE_LIBS ADDITIONAL_DEBUG_LIBS)
                grab_libs(LIST_SHARED_LIBRARIES_RELEASE "${STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_RELEASE}" EXCLUDE_LIBS ADDITIONAL_RELEASE_LIBS)
                set( STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_DEBUG   ${LIST_SHARED_LIBRARIES_DEBUG} )
                set( STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_RELEASE ${LIST_SHARED_LIBRARIES_RELEASE} )
            endif()

            if( LINK_THIRD_PARTY )                 
                MERGE_STATIC_LIBRARIES( ${NAME_MODULE} ALL "${STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}}" )
                MERGE_STATIC_LIBRARIES( ${PROJECT_NAME} DEBUG "${STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_DEBUG}" )
                MERGE_STATIC_LIBRARIES( ${PROJECT_NAME} RELEASE "${STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_RELEASE}" )
            endif()

            target_link_libraries  ( ${NAME_MODULE}  ${STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}}
                                                     ${STATIC_LIBRARIES_SYSTEM_${DAVA_PLATFORM_CURENT}} )  

            foreach ( FILE ${STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_DEBUG} )
                target_link_libraries  ( ${NAME_MODULE} debug ${FILE} )
            endforeach ()

            foreach ( FILE ${STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_RELEASE} )
                target_link_libraries  ( ${NAME_MODULE} optimized ${FILE} )
            endforeach () 

            reset_property( STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT} )
            reset_property( STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_RELEASE )
            reset_property( STATIC_LIBRARIES_${DAVA_PLATFORM_CURENT}_DEBUG )
            reset_property( STATIC_LIBRARIES_SYSTEM_${DAVA_PLATFORM_CURENT} )     
            reset_property( INCLUDES_PRIVATE )            
            reset_property( INCLUDES_PRIVATE_${DAVA_PLATFORM_CURENT} )            

            if ( WINDOWS_UAP )
                set_property(TARGET ${NAME_MODULE} PROPERTY VS_MOBILE_EXTENSIONS_VERSION ${WINDOWS_UAP_MOBILE_EXT_SDK_VERSION} )
            endif()

        endif()
    endif()

endmacro ()



