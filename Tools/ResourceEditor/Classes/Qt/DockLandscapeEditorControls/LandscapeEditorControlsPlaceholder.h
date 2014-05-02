#ifndef __RESOURCEEDITORQT__LANDSCAPEEDITORCONTROLSPLACEHOLDER__
#define __RESOURCEEDITORQT__LANDSCAPEEDITORCONTROLSPLACEHOLDER__

#include <QWidget>
#include "DAVAEngine.h"

#include "LandscapeEditorPanels/LandscapeEditorBasePanel.h"

class CustomColorsPanel;
class RulerToolPanel;
class VisibilityToolPanel;
class TilemaskEditorPanel;
class HeightmapEditorPanel;
class GrassEditorPanel;

class LandscapeEditorControlsPlaceholder: public QWidget
{
	Q_OBJECT
	
public:
	explicit LandscapeEditorControlsPlaceholder(QWidget* parent = 0);
	~LandscapeEditorControlsPlaceholder();
	
	void SetPanel(LandscapeEditorBasePanel* panel);
	void RemovePanel();

private slots:
	void SceneActivated(SceneEditor2* scene);
	void SceneDeactivated(SceneEditor2* scene);

	void EditorToggled(SceneEditor2* scene);

private:
	SceneEditor2* activeScene;
	LandscapeEditorBasePanel* currentPanel;

	CustomColorsPanel* customColorsPanel;
	RulerToolPanel* rulerToolPanel;
	VisibilityToolPanel* visibilityToolPanel;
	TilemaskEditorPanel* tilemaskEditorPanel;
	HeightmapEditorPanel* heightmapEditorPanel;
    GrassEditorPanel *grassEditorPanel;

	void InitUI();
	void ConnectToSignals();
	void CreatePanels();

	void UpdatePanels();
};

#endif /* defined(__RESOURCEEDITORQT__LANDSCAPEEDITORCONTROLSPLACEHOLDER__) */
