#ifndef _QT_CUBEMAPEDITORDIALOG_H_
#define _QT_CUBEMAPEDITORDIALOG_H_

#include <QDialog>
#include <QImage>
#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"

class ClickableQLabel;

namespace Ui {
class CubemapEditorDialog;
}

class CubemapEditorDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit CubemapEditorDialog(QWidget *parent = 0);
    ~CubemapEditorDialog();
	
	void InitForEditing(DAVA::FilePath& textureDescriptorPath, DAVA::FilePath& rootPath);
	void InitForCreating(DAVA::FilePath& textureDescriptorPath, DAVA::FilePath& rootPath);
	
protected:
	
	typedef enum{
		eEditorModeNone,
		eEditorModeEditing,
		eEditorModeCreating
	} eEditorMode;
	
protected:
	
	float faceWidth;
	float faceHeight;
	QString* facePath;
	QString rootPath;
	
	bool edited;
	
	eEditorMode editorMode;
	DAVA::FilePath targetFile;
	
protected:
	
	void ConnectSignals();
	void LoadImageFromUserFile(float rotation, int face);
	bool VerifyImage(const QImage& image, int faceIndex);
	void UpdateFaceInfo();
	void UpdateButtonState();
	bool AnyFaceLoaded();
	bool AllFacesLoaded();
	int GetLoadedFaceCount();
	void LoadCubemap(const QString& path);
	void SaveCubemap(const QString& path);
	DAVA::uint8 GetFaceMask();
	void LoadImageTo(const DAVA::String& filePath, int face, bool silent);
	ClickableQLabel* GetLabelForFace(int face);
	
protected slots:

	void OnPXClicked();
	void OnNXClicked();
	void OnPYClicked();
	void OnNYClicked();
	void OnPZClicked();
	void OnNZClicked();
	
	void OnLoadTexture();
	void OnSave();
	void OnClose();
    
private:
    Ui::CubemapEditorDialog *ui;
};

#endif // _QT_CUBEMAPEDITORDIALOG_H_
