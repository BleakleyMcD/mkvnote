#pragma once

#include <QMainWindow>
#include <QMap>
#include <QStringList>
#include <QLineEdit>
#include <QTextEdit>
#include <QProcess>
#include <QTemporaryFile>
#include <QFileInfo>
#include <QLabel>
#include <QDragEnterEvent>
#include <QDropEvent>

class QScrollArea;
class QFormLayout;
class QWidget;
class QAbstractScrollArea;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &inputFile = QString(), QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onTagOn();
    void onCancel();
    void onRevert();
    void onOpenFile();

private:
    // tag definitions 
    const QStringList NMAAHC_TAG_SET = {
        "COLLECTION", "TITLE", "CATALOG_NUMBER", "DESCRIPTION",
        "DATE_DIGITIZED", "ENCODER_SETTINGS", "ENCODED_BY",
        "ORIGINAL_MEDIA_TYPE", "DATE_TAGGED", "_TAGGED_BY",
        "TERMS_OF_USE", "_PRE_TRANSFER_NOTES", "_TRANSFER_NOTES", "_ORIGINAL_FPS"
    };
    const QStringList RO_TAGS        = { "ENCODER", "VIDEO_STREAM_HASH", "AUDIO_STREAM_HASH" };
    const QStringList MULTILINE_TAGS = { "DESCRIPTION", "ENCODER_SETTINGS", "_PRE_TRANSFER_NOTES", "_TRANSFER_NOTES" };
    const QMap<QString,int> FIELD_HEIGHTS = {
        {"DESCRIPTION", 105}, {"ENCODER_SETTINGS", 90},
        {"_PRE_TRANSFER_NOTES", 60}, {"_TRANSFER_NOTES", 60}
    };

    // state 
    QString                  m_inputFile;
    QString                  m_logFile;
    QStringList              m_tagOrder;
    QMap<QString, QWidget*>  m_fields;
    QWidget                 *m_formContainer = nullptr;
    QFormLayout             *m_formLayout    = nullptr;
    QLabel                  *m_dropHint      = nullptr;
    QScrollArea             *m_scrollArea    = nullptr;

    // helpers 
    void                     loadFile(const QString &path);
    void                     buildForm(QWidget *container, QFormLayout *layout);
    void                     populateForm(const QMap<QString, QString> &tags);
    void                     clearForm();
    QMap<QString, QString>   extractExistingTags();
    QString                  getAttachments();
    bool                     writeTagsToMkv(const QMap<QString, QString> &tags);
    void                     initLog();
    void                     log(const QString &level, const QString &message);
    void                     logXml(const QString &xmlContent);
    QLabel                  *makeSectionHeader(const QString &text);
};
