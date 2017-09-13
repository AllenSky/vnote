#include "vfile.h"

#include <QDir>
#include <QDebug>
#include <QTextEdit>
#include <QFileInfo>
#include "utils/vutils.h"

VFile::VFile(const QString &p_name, QObject *p_parent,
             FileType p_type, bool p_modifiable)
    : QObject(p_parent), m_name(p_name), m_opened(false), m_modified(false),
      m_docType(VUtils::docTypeFromName(p_name)),
      m_type(p_type), m_modifiable(p_modifiable)
{
}

VFile::~VFile()
{
}

bool VFile::open()
{
    Q_ASSERT(!m_name.isEmpty());
    if (m_opened) {
        return true;
    }
    Q_ASSERT(m_content.isEmpty());
    QString path = fetchPath();
    qDebug() << "path" << path;
    m_content = VUtils::readFileFromDisk(path);
    m_modified = false;
    m_opened = true;
    qDebug() << "file" << m_name << "opened";
    return true;
}

void VFile::close()
{
    if (!m_opened) {
        return;
    }
    m_content.clear();
    m_opened = false;
}

void VFile::deleteDiskFile()
{
    V_ASSERT(parent());

    // Delete local images if it is Markdown.
    if (m_docType == DocType::Markdown) {
        deleteLocalImages();
    }

    // Delete the file
    QString filePath = fetchPath();
    QFile file(filePath);
    if (file.remove()) {
        qDebug() << "deleted" << filePath;
    } else {
        qWarning() << "fail to delete" << filePath;
    }
}

bool VFile::save()
{
    Q_ASSERT(m_opened);
    bool ret = VUtils::writeFileToDisk(fetchPath(), m_content);
    return ret;
}

void VFile::setModified(bool p_modified)
{
    m_modified = p_modified;
}

void VFile::deleteLocalImages()
{
    V_ASSERT(m_docType == DocType::Markdown);

    QVector<ImageLink> images = VUtils::fetchImagesFromMarkdownFile(this,
                                                                    ImageLink::LocalRelativeInternal);
    int deleted = 0;
    for (int i = 0; i < images.size(); ++i) {
        QFile file(images[i].m_path);
        if (file.remove()) {
            ++deleted;
        }
    }

    qDebug() << "delete" << deleted << "images for" << fetchPath();
}

void VFile::setName(const QString &p_name)
{
    m_name = p_name;
    DocType newType = VUtils::docTypeFromName(p_name);
    if (newType != m_docType) {
        qWarning() << "setName() change the DocType. A convertion should be followed";
    }
}

const QString &VFile::getName() const
{
    return m_name;
}

VDirectory *VFile::getDirectory()
{
    Q_ASSERT(parent());
    return (VDirectory *)parent();
}

const VDirectory *VFile::getDirectory() const
{
    Q_ASSERT(parent());
    return (const VDirectory *)parent();
}

DocType VFile::getDocType() const
{
    return m_docType;
}

const QString &VFile::getContent() const
{
    return m_content;
}

QString VFile::getNotebookName() const
{
    return getDirectory()->getNotebookName();
}

const VNotebook *VFile::getNotebook() const
{
    return getDirectory()->getNotebook();
}

VNotebook *VFile::getNotebook()
{
    return getDirectory()->getNotebook();
}

QString VFile::fetchPath() const
{
    QString dirPath = getDirectory()->fetchPath();
    return QDir(dirPath).filePath(m_name);
}

QString VFile::fetchRelativePath() const
{
    QString dirRelativePath = getDirectory()->fetchRelativePath();
    return QDir(dirRelativePath).filePath(m_name);
}

QString VFile::fetchBasePath() const
{
    return getDirectory()->fetchPath();
}

QString VFile::fetchImagePath() const
{
    return QDir(fetchBasePath()).filePath(getNotebook()->getImageFolder());
}

void VFile::setContent(const QString &p_content)
{
    m_content = p_content;
}

bool VFile::isModified() const
{
    return m_modified;
}

bool VFile::isModifiable() const
{
    return m_modifiable;
}

bool VFile::isOpened() const
{
    return m_opened;
}

FileType VFile::getType() const
{
    return m_type;
}

bool VFile::isInternalImageFolder(const QString &p_path) const
{
    return VUtils::equalPath(VUtils::basePathFromPath(p_path),
                             getDirectory()->fetchPath());
}

QUrl VFile::getBaseUrl() const
{
    // Need to judge the path: Url, local file, resource file.
    QUrl baseUrl;
    QString basePath = fetchBasePath();
    QFileInfo pathInfo(basePath);
    if (pathInfo.exists()) {
        if (pathInfo.isNativePath()) {
            // Local file.
            baseUrl = QUrl::fromLocalFile(basePath + QDir::separator());
        } else {
            // Resource file.
            baseUrl = QUrl("qrc" + basePath + QDir::separator());
        }
    } else {
        // Url.
        baseUrl = QUrl(basePath + QDir::separator());
    }

    return baseUrl;
}

bool VFile::rename(const QString &p_name)
{
    if (m_name == p_name) {
        return true;
    }

    QString oldName = m_name;

    VDirectory *dir = getDirectory();
    V_ASSERT(dir);
    // Rename it in disk.
    QDir diskDir(dir->fetchPath());
    if (!diskDir.rename(m_name, p_name)) {
        qWarning() << "fail to rename note" << m_name << "to" << p_name << "in disk";
        return false;
    }

    m_name = p_name;

    // Update parent directory's config file.
    if (!dir->writeToConfig()) {
        m_name = oldName;
        diskDir.rename(p_name, m_name);
        return false;
    }

    // Can't not change doc type.
    Q_ASSERT(m_docType == VUtils::docTypeFromName(m_name));

    qDebug() << "note renamed from" << oldName << "to" << m_name;

    return true;
}

bool VFile::isRelativeImageFolder() const
{
    return true;
}

QString VFile::getImageFolderInLink() const
{
    return getNotebook()->getImageFolder();
}
