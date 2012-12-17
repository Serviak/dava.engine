//
//  StringUtils.h
//  UIEditor
//
//  Created by Yuri Coder on 11/16/12.
//
//

#ifndef __UIEditor__StringUtils__
#define __UIEditor__StringUtils__

#include <QString>

namespace DAVA {
    
// Different string utilities.
// Truncate the file extension.
QString TruncateFileExtension(const QString& fileName, const QString& extension);
    
// Truncate the ".txt" file extension.
QString TruncateTxtFileExtension(const QString& fileName);
    
};

#endif /* defined(__UIEditor__Utils__) */
