cd ..\_ReleaseQt

if exist app rmdir /s /q app
if exist app.zip del /q app.zip

mkdir app\ResourceEditor\dava.framework\Tools\ResourceEditor\Data
mkdir app\ResourceEditor\dava.framework\Tools\ResourceEditor\platforms

mkdir app\ResourceEditor\dava.resourceeditor.beast
call git log --since=3.days --branches="development" --pretty=format:"%%%%s (%%%%an, %%%%ar) " >> app/ResourceEditor/changes.txt
echo cd .\dava.framework\Tools\ResourceEditor > app/ResourceEditor/start.cmd
echo start ResourceEditor.exe >> app/ResourceEditor/start.cmd
           
xcopy /e ..\Data\*.* app\ResourceEditor\dava.framework\Tools\ResourceEditor\Data 
xcopy *.exe app\ResourceEditor\dava.framework\Tools\ResourceEditor
xcopy *.pdb app\ResourceEditor\dava.framework\Tools\ResourceEditor
xcopy ..\glew32.dll app\ResourceEditor\dava.framework\Tools\ResourceEditor
xcopy ..\Teamcity\imagesplitter\*.bat app\ResourceEditor\dava.framework\Tools\ResourceEditor

xcopy %QT5_HOME%\bin\Qt5Core.dll app\ResourceEditor\dava.framework\Tools\ResourceEditor
xcopy %QT5_HOME%\bin\Qt5Gui.dll app\ResourceEditor\dava.framework\Tools\ResourceEditor
xcopy %QT5_HOME%\bin\Qt5Widgets.dll app\ResourceEditor\dava.framework\Tools\ResourceEditor
xcopy %QT5_HOME%\bin\icudt52.dll app\ResourceEditor\dava.framework\Tools\ResourceEditor
xcopy %QT5_HOME%\bin\icuin52.dll app\ResourceEditor\dava.framework\Tools\ResourceEditor
xcopy %QT5_HOME%\bin\icuuc52.dll app\ResourceEditor\dava.framework\Tools\ResourceEditor
xcopy %QT5_HOME%\plugins\platforms\qwindows.dll app\ResourceEditor\dava.framework\Tools\ResourceEditor\platforms

xcopy ..\..\..\..\dava.resourceeditor.beast\beast\bin\beast32.dll app\ResourceEditor\dava.framework\Tools\ResourceEditor
xcopy /e ..\..\..\..\dava.resourceeditor.beast\*.* app\ResourceEditor\dava.resourceeditor.beast\
xcopy ..\..\..\..\dava.framework\Libs\fmod\bin\fmodex.dll app\ResourceEditor\dava.framework\Tools\ResourceEditor
xcopy ..\..\..\..\dava.framework\Libs\fmod\bin\fmod_event.dll app\ResourceEditor\dava.framework\Tools\ResourceEditor
xcopy ..\..\..\..\dava.framework\Tools\Bin\IMagickHelper.dll app\ResourceEditor\dava.framework\Tools\ResourceEditor

wzzip -p -r %1 app\*.*	