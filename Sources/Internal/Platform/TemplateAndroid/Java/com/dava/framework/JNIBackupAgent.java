package com.dava.framework;

import android.app.backup.BackupAgentHelper;
import android.app.backup.BackupManager;
import android.app.backup.RestoreObserver;
import android.app.backup.SharedPreferencesBackupHelper;

public class JNIBackupAgent extends BackupAgentHelper {
	public void onCreate() {
        SharedPreferencesBackupHelper helper = new SharedPreferencesBackupHelper(this, JNISharedPreferences.GetName());
        addHelper(JNISharedPreferences.GetName(), helper);
    }
	
	static void Backup()
	{
		BackupManager bm = new BackupManager(JNIApplication.GetApplication().getApplicationContext());
	    bm.dataChanged();
	}
	
	// You should to put observer if you want to restore settings menually
	static void Restore(RestoreObserver observer)
	{
		BackupManager bm = new BackupManager(JNIApplication.GetApplication().getApplicationContext());
	    bm.requestRestore(observer);
	}

}
