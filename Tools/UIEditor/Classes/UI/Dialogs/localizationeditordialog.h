#ifndef LOCALIZATIONEDITORDIALOG_H
#define LOCALIZATIONEDITORDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include <QItemSelection>

namespace Ui {
class LocalizationEditorDialog;
}
    
class LocalizationEditorDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit LocalizationEditorDialog(QWidget *parent = 0);
    ~LocalizationEditorDialog();
    
protected:
	// Setup the Localization Table Model.
	void SetupLocalizationTable();
	
	// Reload the Localization Table each time Current Locale is changed.
	void ReloadLocalizationTable();

	// Cleanup the UI controls related to selection.
	void CleanupLocalizationUIControls();

	// Store the changes for the string being deselected.
	void ProcessDeselectedString(const QItemSelection & deselected);
	
	// Update the UI with the string currently selected.
	void ProcessSelectedString(const QItemSelection & selected);
	
	// Update the Localization System with key and value currently set.
	void UpdateLocalizationValueForCurrentKey();
	void UpdateLocalizationValueForCurrentKey(const QModelIndex& selectedItemIndex);

	virtual void closeEvent(QCloseEvent *event);

	// Add/remove localization string.
	void AddNewLocalizationString();
	void RemoveSelectedLocalizationString();

	// Select the item in table by its key.
	void SelectItemByKey(const QString& keyToBeSelected);

private:
	static const int LOCALIZATION_KEY_INDEX;
	static const int LOCALIZATION_VALUE_INDEX;
	
	static const QString DEFAULT_LOCALIZATION_KEY;
	static const QString DEFAULT_LOCALIZATION_VALUE;

    Ui::LocalizationEditorDialog *ui;
    void FillLocaleComboBox();
    void ConnectToSignals();
    void SetLocalizationDirectoryPath();
    void SetDefaultLanguage();

    void ReinitializeLocalizationSystem(const QString& localizationDirectory);

	void ApplySortOrder(int headerIndex);

private slots:
    void OnOpenLocalizationFileButtonClicked();
    void OnCurrentLocaleChanged(int index);
  
	// A string in the localization table is selected.
	void OnLocalizationStringSelected(const QItemSelection & selected, const QItemSelection & deselected);
	
	// Table View header is clicked. Needed for sorting.
	void OnTableHeaderClicked(int headerIndex);

	// Custom menu needs to be displayed.
	void OnShowCustomMenu(const QPoint& pos);

	// An localization string should be added/removed.
	void OnAddLocalizationStringAction();
	void OnRemoveLocalizationStringAction();

	// Save the content of the Localization System.
	void SaveLocalization();

	// A dialog is closed.
	void CloseDialog();

private:
	// Localization Table Model.
	QStandardItemModel* tableModel;
	
	// Current sort order.
	Qt::SortOrder sortOrder;

	// Global index for the strings currently added.
	static int addedStringsCount;
};

#endif // LOCALIZATIONEDITORDIALOG_H
