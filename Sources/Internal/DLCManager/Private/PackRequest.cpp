#include "DLCManager/Private/PackRequest.h"
#include "DLCManager/Private/DLCManagerImpl.h"
#include "FileSystem/Private/PackMetaData.h"
#include "FileSystem/FileSystem.h"
#include "Utils/CRC32.h"
#include "Logger/Logger.h"
#include "Compression/Compressor.h"

#include <numeric>

namespace DAVA
{
const String extPart(".part");

PackRequest::PackRequest(DLCManagerImpl& packManager_, const String& pack_)
    : packManagerImpl(&packManager_)
    , requestedPackName(pack_)
    , delayedRequest(true)
{
}

PackRequest::PackRequest(DLCManagerImpl& packManager_, const String& pack_, Vector<uint32> fileIndexes_)
    : packManagerImpl(&packManager_)
    , requestedPackName(pack_)
    , delayedRequest(false)
{
    SetFileIndexes(move(fileIndexes_));
}

void PackRequest::CancelCurrentDownloadRequests()
{
    DLCDownloader& downloader = packManagerImpl->GetDownloader();

    for (FileRequest& r : requests)
    {
        if (r.task != nullptr)
        {
            downloader.RemoveTask(r.task);
            r.task = nullptr;
        }
    }

    requests.clear();
    requests.shrink_to_fit();

    fileRequestsInitialized = false;
}

PackRequest::~PackRequest()
{
    CancelCurrentDownloadRequests();
    packManagerImpl = nullptr;
    requests.clear();
    fileIndexes.clear();
    requestedPackName.clear();
    numOfDownloadedFile = 0;
    delayedRequest = false;
}

void PackRequest::Start()
{
    // just continue call Update
}

void PackRequest::Stop()
{
    CancelCurrentDownloadRequests();
    numOfDownloadedFile = 0;
}

const String& PackRequest::GetRequestedPackName() const
{
    return requestedPackName;
}

Vector<uint32> PackRequest::GetDependencies() const
{
    if (dependencyCache.capacity() > 0)
    {
        return dependencyCache;
    }
    if (packManagerImpl->IsInitialized())
    {
        const PackMetaData& pack_meta_data = packManagerImpl->GetMeta();
        dependencyCache = pack_meta_data.GetPackDependencyIndexes(requestedPackName);
        if (dependencyCache.capacity() == 0)
        {
            dependencyCache.reserve(1); // just mark to know we already check it
        }
        return dependencyCache;
    }
    DAVA_THROW(Exception, "Error! Can't get pack dependencies before initialization is finished");
}
/** return size of files within this request without dependencies */
uint64 PackRequest::GetSize() const
{
    uint64 allFilesSize = 0;
    const auto& files = packManagerImpl->GetPack().filesTable.data.files;
    for (uint32 fileIndex : fileIndexes)
    {
        const auto& fileInfo = files.at(fileIndex);
        allFilesSize += (fileInfo.compressedSize + sizeof(PackFormat::LitePack::Footer));
    }
    return allFilesSize;
}
/** recalculate current downloaded size without dependencies */
uint64 PackRequest::GetDownloadedSize() const
{
    uint64 requestsSize = std::accumulate(begin(requests), end(requests), uint64(0), [](uint64 sum, const FileRequest& r) {
        return sum + r.downloadedFileSize;
    });
    return requestsSize;
}

/** return true when all files loaded and ready */
bool PackRequest::IsDownloaded() const
{
    if (delayedRequest)
    {
        return false;
    }

    if (!fileRequestsInitialized)
    {
        return false;
    }

    if (!packManagerImpl->IsInitialized())
    {
        return false;
    }

    for (const FileRequest& r : requests)
    {
        if (r.status != Ready)
        {
            return false;
        }
    }

    if (packManagerImpl->IsInQueue(this))
    {
        if (!packManagerImpl->IsTop(this))
        {
            // wait for dependencies to download first
            return false;
        }
    }

    return true;
}

void PackRequest::SetFileIndexes(Vector<uint32> fileIndexes_)
{
    fileIndexes = std::move(fileIndexes_);
    delayedRequest = false;
}

bool PackRequest::IsSubRequest(const PackRequest* other) const
{
    const auto& meta = packManagerImpl->GetMeta();
    uint32 thisPackIndex = meta.GetPackIndex(requestedPackName);
    uint32 childPackIndex = meta.GetPackIndex(other->requestedPackName);
    return meta.IsChild(thisPackIndex, childPackIndex);
}

void PackRequest::InitializeFileRequests()
{
    DVASSERT(fileRequestsInitialized == false);
    // cancel all correctly if start/stop request happened
    CancelCurrentDownloadRequests();
    requests.reserve(fileIndexes.size());

    const String& url = packManagerImpl->GetSuperPackUrl();

    for (size_t requestIndex = 0; requestIndex < fileIndexes.size(); ++requestIndex)
    {
        uint32 fileIndex = fileIndexes.at(requestIndex);
        const auto& fileInfo = packManagerImpl->GetPack().filesTable.data.files.at(fileIndex);
        String relativePath = packManagerImpl->GetRelativeFilePath(fileIndex);
        FilePath localPath = packManagerImpl->GetLocalPacksDirectory() + relativePath + extDvpl + extPart;

        requests.emplace_back(localPath,
                              url,
                              fileIndex,
                              fileInfo.compressedCrc32,
                              fileInfo.startPosition,
                              fileInfo.compressedSize,
                              fileInfo.originalSize,
                              nullptr,
                              fileInfo.type,
                              Wait);
    }

    fileRequestsInitialized = true;
}

bool PackRequest::Update()
{
    DVASSERT(Thread::IsMainThread());
    DVASSERT(packManagerImpl->IsInitialized());

    bool needFireUpdateSignal = false;

    if (!fileRequestsInitialized)
    {
        InitializeFileRequests();
        packManagerImpl->requestStartLoading.Emit(*this);
    }

    if (!IsDownloaded())
    {
        needFireUpdateSignal = UpdateFileRequests();
    }

    return needFireUpdateSignal;
}

PackRequest::FileRequest::FileRequest(FilePath localFile_,
                                      String url_,
                                      uint32 fileIndex_,
                                      uint32 hashFromMeta_,
                                      uint64 startLoadingPos_,
                                      uint64 sizeOfCompressedFile_,
                                      uint64 sizeOfUncompressedFile_,
                                      DLCDownloader::Task* task_,
                                      Compressor::Type compressionType_,
                                      Status status_)
    : localFile(localFile_)
    , url(url_)
    , fileIndex(fileIndex_)
    , compressedCrc32(hashFromMeta_)
    , startLoadingPos(startLoadingPos_)
    , sizeOfCompressedFile(sizeOfCompressedFile_)
    , sizeOfUncompressedFile(sizeOfUncompressedFile_)
    , downloadedFileSize(0)
    , task(task_)
    , compressionType(compressionType_)
    , status(status_)
{
}

PackRequest::FileRequest::~FileRequest()
{
    // just for safe clear all fields
    localFile = FilePath();
    url.clear();
    fileIndex = 0;
    compressedCrc32 = 0;
    startLoadingPos = 0;
    sizeOfCompressedFile = 0;
    sizeOfUncompressedFile = 0;
    downloadedFileSize = 0;
    task = nullptr;
    compressionType = Compressor::Type::Lz4HC;
    status = Wait;
}

void PackRequest::DeleteJustDownloadedFileAndStartAgain(FileRequest& fileRequest)
{
    fileRequest.downloadedFileSize = 0;
    FileSystem* fs = GetEngineContext()->fileSystem;
    bool deleteOk = fs->DeleteFile(fileRequest.localFile);
    if (!deleteOk)
    {
        Logger::Error("DLCManager can't delete invalid file: %s", fileRequest.localFile.GetStringValue().c_str());
    }
    fileRequest.status = LoadingPackFile;
}

void PackRequest::DisableRequestingAndFireSignalIOError(FileRequest& fileRequest, int32 errVal, const String& extMsg) const
{
    String pathname = fileRequest.localFile.GetAbsolutePathname();

    std::ostream& log = packManagerImpl->GetLog();
    log << "device IO Error:(" << errVal << ") "
        << strerror(errVal) << " file: " << pathname
        << " extended_message: " << extMsg << std::endl;

    bool fireSignal = packManagerImpl->CountError(errVal);

    if (fireSignal)
    {
        log << " disable DLCManager requesting" << std::endl;
        packManagerImpl->SetRequestingEnabled(false);
        packManagerImpl->fileErrorOccured.Emit(pathname.c_str(), errVal);
    }
    else
    {
        DeleteJustDownloadedFileAndStartAgain(fileRequest);
    }
}

bool PackRequest::CheckLocalFileState(FileSystem* fs, FileRequest& fileRequest)
{
    if (packManagerImpl->IsFileReady(fileRequest.fileIndex))
    {
        fileRequest.status = Ready;
        uint64 fileSize = 0;
        FilePath dvplPath = fileRequest.localFile;
        dvplPath.ReplaceExtension("");
        if (fs->GetFileSize(dvplPath, fileSize))
        {
            if (fileSize == fileRequest.sizeOfCompressedFile + sizeof(PackFormat::LitePack::Footer))
            {
                fileRequest.downloadedFileSize = fileSize;
                return true;
            }
            // file exist but may be invalid so let's check it out
            fileRequest.status = CheckHash;
            return false;
        }
    }
    else
    {
        fileRequest.status = LoadingPackFile;
    }
    return false;
}

bool PackRequest::CheckLoadingStatusOfFileRequest(FileRequest& fileRequest, DLCDownloader& dm, const String& dstPath)
{
    if (fileRequest.task == nullptr)
    {
        return false;
    }

    DLCDownloader::TaskStatus status = dm.GetTaskStatus(fileRequest.task);
    {
        switch (status.state)
        {
        case DLCDownloader::TaskState::JustAdded:
            break;
        case DLCDownloader::TaskState::Downloading:
        {
            if (fileRequest.downloadedFileSize != status.sizeDownloaded)
            {
                fileRequest.downloadedFileSize = status.sizeDownloaded;
                return true;
            }
        }
        break;
        case DLCDownloader::TaskState::Finished:
        {
            dm.RemoveTask(fileRequest.task);
            fileRequest.task = nullptr;

            bool allGood = !status.error.errorHappened;

            if (allGood)
            {
                fileRequest.task = nullptr;
                fileRequest.downloadedFileSize = status.sizeDownloaded;
                fileRequest.status = CheckHash;
                return true;
            }

            std::ostream& out = packManagerImpl->GetLog();

            out << "file_request failed: can't download file: " << dstPath << " status: " << status << std::endl;

            if (status.error.fileErrno != 0)
            {
                out << " I/O error: " << status.error.errStr << std::endl;
                DisableRequestingAndFireSignalIOError(fileRequest, status.error.fileErrno, "task_finished_see_dlc_manager_log");
                return false;
            }

            DeleteJustDownloadedFileAndStartAgain(fileRequest);
            return false;
        }
        }
    }
    return false;
}

bool PackRequest::LoadingPackFileState(FileSystem* fs, FileRequest& fileRequest)
{
    DLCDownloader& dm = packManagerImpl->GetDownloader();
    String dstPath = fileRequest.localFile.GetAbsolutePathname();
    if (fileRequest.task == nullptr)
    {
        if (fileRequest.sizeOfCompressedFile == 0)
        {
            // just create empty file, and go to next state
            FilePath dirPath = fileRequest.localFile.GetDirectory();
            FileSystem::eCreateDirectoryResult dirCreate = fs->CreateDirectory(dirPath, true);
            if (dirCreate == FileSystem::DIRECTORY_CANT_CREATE)
            {
                DisableRequestingAndFireSignalIOError(fileRequest, errno, "can_t_create_directory: " + dirPath.GetAbsolutePathname());
                return false;
            }
            ScopedPtr<File> f(File::Create(fileRequest.localFile, File::CREATE | File::WRITE));
            if (!f)
            {
                DisableRequestingAndFireSignalIOError(fileRequest, errno, "can_t_create_local_file");
                return false;
            }
            if (!f->Truncate(0))
            {
                DisableRequestingAndFireSignalIOError(fileRequest, errno, "can_t_truncate_local_file");
                return false;
            }
            fileRequest.task = nullptr;
            fileRequest.status = CheckHash;
            return true;
        }

        DLCDownloader::Range range = DLCDownloader::Range(fileRequest.startLoadingPos, fileRequest.sizeOfCompressedFile);
        fileRequest.task = dm.ResumeTask(fileRequest.url, dstPath, range);
        if (nullptr == fileRequest.task)
        {
            Logger::Error("can't create task: url: %s, dstPath: %s, range: %lld-%lld", fileRequest.url.c_str(), dstPath.c_str(), range.size, range.offset);
            fileRequest.status = Wait; // lets start all over again
        }
        return false;
    }

    return CheckLoadingStatusOfFileRequest(fileRequest, dm, dstPath);
}

bool PackRequest::CheckHaskState(FileRequest& fileRequest)
{
    FileSystem* fs = GetEngineContext()->fileSystem;
    uint64 fileSize = 0;
    fs->GetFileSize(fileRequest.localFile, fileSize);
    uint32 fileCrc32 = CRC32::ForFile(fileRequest.localFile);

    if (fileCrc32 == fileRequest.compressedCrc32 && fileSize == fileRequest.sizeOfCompressedFile)
    {
        // write 20 bytes LitePack footer
        PackFormat::LitePack::Footer footer;
        footer.type = fileRequest.compressionType;
        footer.crc32Compressed = fileRequest.compressedCrc32;
        footer.sizeUncompressed = static_cast<uint32>(fileRequest.sizeOfUncompressedFile);
        footer.sizeCompressed = static_cast<uint32>(fileRequest.sizeOfCompressedFile);
        footer.packMarkerLite = PackFormat::FILE_MARKER_LITE;

        {
            ScopedPtr<File> f(File::Create(fileRequest.localFile, File::WRITE | File::APPEND));
            if (!f)
            {
                DisableRequestingAndFireSignalIOError(fileRequest, errno, "can_t_open_local_file_for_append");
                return false;
            }

            uint32 written = f->Write(&footer, sizeof(footer));
            if (written != sizeof(footer))
            {
                // not enough space
                DisableRequestingAndFireSignalIOError(fileRequest, errno, "can_t_write_footer_to_local_file");
                return false;
            }
        }
        // rename file from "xxx.dvpl.part" to "xxx.dvpl"
        {
            DVASSERT(fileRequest.localFile.GetExtension() == ".part");

            FilePath newPath(fileRequest.localFile);
            newPath.ReplaceExtension("");

            DVASSERT(newPath.GetExtension() == extDvpl);

            if (!fs->MoveFile(fileRequest.localFile, newPath, true))
            {
                DisableRequestingAndFireSignalIOError(fileRequest, errno, "can_t_rename_local_file");
                return false;
            }
        }

        fileRequest.downloadedFileSize += sizeof(footer);
        fileRequest.status = Ready;

        packManagerImpl->SetFileIsReady(fileRequest.fileIndex);

        return true;
    }

    // try download again
    DeleteJustDownloadedFileAndStartAgain(fileRequest);
    return false;
}

bool PackRequest::UpdateFileRequests()
{
    // return true if at least one part file continue downloading
    bool callUpdateSignal = false;

    FileSystem* fs = GetEngineContext()->fileSystem;

    for (FileRequest& fileRequest : requests)
    {
        bool downloadedMore = false;
        switch (fileRequest.status)
        {
        case Wait:
        {
            fileRequest.status = CheckLocalFile;
            break;
        }
        case CheckLocalFile:
        {
            downloadedMore = CheckLocalFileState(fs, fileRequest);
            break;
        }
        case LoadingPackFile:
        {
            downloadedMore = LoadingPackFileState(fs, fileRequest);
            break;
        }
        case CheckHash:
        {
            downloadedMore = CheckHaskState(fileRequest);
            break;
        }
        case Ready:
            break;
        case Error:
            break;
        } // end switch

        if (downloadedMore)
        {
            callUpdateSignal = true;
        }
    } // end for requests

    // call signal only once during update
    return callUpdateSignal;
}

} // end namespace DAVA
