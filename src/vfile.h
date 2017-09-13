#ifndef VFILE_H
#define VFILE_H

#include <QObject>
#include <QString>
#include <QUrl>
#include "vdirectory.h"
#include "vconstants.h"

class VNotebook;

class VFile : public QObject
{
    Q_OBJECT
public:
    VFile(const QString &p_name, QObject *p_parent,
          FileType p_type = FileType::Normal, bool p_modifiable = true);
    virtual ~VFile();
    virtual bool open();
    virtual void close();
    virtual bool save();

    const QString &getName() const;
    virtual void setName(const QString &p_name);
    virtual VDirectory *getDirectory();
    virtual const VDirectory *getDirectory() const;
    DocType getDocType() const;
    const QString &getContent() const;
    virtual void setContent(const QString &p_content);
    virtual const VNotebook *getNotebook() const;
    virtual VNotebook *getNotebook();
    virtual QString getNotebookName() const;
    virtual QString fetchPath() const;
    virtual QString fetchRelativePath() const;
    virtual QString fetchBasePath() const;

    // The path of the image folder.
    virtual QString fetchImagePath() const;

    bool isModified() const;
    bool isModifiable() const;
    bool isOpened() const;
    FileType getType() const;

    // Return the base URL for this file when loaded in VWebView.
    QUrl getBaseUrl() const;

    // Whether the directory @p_path is an internal image folder of this file.
    // It is true only when the folder is in the same directory as the parent
    // directory of this file.
    virtual bool isInternalImageFolder(const QString &p_path) const;

    // Rename the file.
    virtual bool rename(const QString &p_name);

    // Whether the image folder is a relative path.
    virtual bool isRelativeImageFolder() const;

    // Return the image folder part in an image link.
    virtual QString getImageFolderInLink() const;

public slots:
    void setModified(bool p_modified);

protected:
    // Delete the file and corresponding images
    void deleteDiskFile();

    // Delete local images of DocType::Markdown.
    void deleteLocalImages();

    QString m_name;
    bool m_opened;
    // File has been modified in editor
    bool m_modified;
    DocType m_docType;
    QString m_content;
    FileType m_type;
    bool m_modifiable;

    friend class VDirectory;
};

#endif // VFILE_H
