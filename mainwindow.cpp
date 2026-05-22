#include "mainwindow.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QScrollArea>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QWidget>
#include <QProcess>
#include <QTemporaryFile>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDateTime>
#include <QMessageBox>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDir>
#include <QDebug>
#include <QTime>
#include <QMenuBar>
#include <QAction>
#include <QFileDialog>
#include <QMimeData>
#include <QUrl>
#include <QSizePolicy>

MainWindow::MainWindow(const QString &inputFile, QWidget *parent)
    : QMainWindow(parent)
{
    setAcceptDrops(true);
    setMinimumSize(700, 500);
    resize(800, 700);

    // menu bar
    QMenu *fileMenu = menuBar()->addMenu("File");
    QAction *openAct = fileMenu->addAction("Open...");
    openAct->setShortcut(QKeySequence::Open);
    connect(openAct, &QAction::triggered, this, &MainWindow::onOpenFile);

    //central widget 
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    central->setStyleSheet("background-color: #2b2b2b; color: white;");

    QVBoxLayout *outerLayout = new QVBoxLayout(central);
    outerLayout->setContentsMargins(12, 12, 12, 12);
    outerLayout->setSpacing(8);

    //title
    QLabel *title = new QLabel("NMAAHC mkvnote — Matroska Metadata Editor");
    title->setStyleSheet("font-size: 16px; font-weight: bold; color: #C97FD4;");
    outerLayout->addWidget(title);

    //description 
    QLabel *desc = new QLabel(
        "Edit metadata tags. These tags semantically describe the file as a whole "
        "and are not intended to refer to a particular track or attachment. "
        "Empty tags will be ignored. Existing tags will be overwritten when saved.");
    desc->setWordWrap(true);
    desc->setStyleSheet("color: white;");
    outerLayout->addWidget(desc);

    //drop hint 
    m_dropHint = new QLabel("Drop a Matroska here\nor use File > Open…");
    m_dropHint->setAlignment(Qt::AlignCenter);
    m_dropHint->setStyleSheet("color: #aaa; font-size: 18px; padding: 60px;");
    outerLayout->addWidget(m_dropHint, 1);

    //scroll area 
    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");
    m_scrollArea->hide();
    outerLayout->addWidget(m_scrollArea, 1);

    m_formContainer = new QWidget;
    m_formContainer->setStyleSheet("background-color: #2b2b2b;");
    m_formContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_scrollArea->setWidget(m_formContainer);

    m_formLayout = new QFormLayout(m_formContainer);
    m_formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    m_formLayout->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_formLayout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_formLayout->setHorizontalSpacing(16);
    m_formLayout->setVerticalSpacing(6);

    //button row 
    QHBoxLayout *btnRow = new QHBoxLayout;
    QPushButton *btnTagOn  = new QPushButton("Tag-On!");
    QPushButton *btnRevert = new QPushButton("Revert");
    QPushButton *btnCancel = new QPushButton("Cancel");

    btnTagOn->setDefault(true);
    btnTagOn->setStyleSheet(
        "QPushButton { background-color: #7B2891; color: white; "
        "padding: 6px 20px; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #9A35B5; }");
    btnRevert->setStyleSheet(
        "QPushButton { background-color: #888; color: white; "
        "padding: 6px 20px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #aaa; }");
    btnCancel->setStyleSheet(
        "QPushButton { background-color: #555; color: white; "
        "padding: 6px 20px; border-radius: 4px; }"
        "QPushButton:hover { background-color: #777; }");

    btnRow->addStretch();
    btnRow->addWidget(btnTagOn);
    btnRow->addWidget(btnRevert);
    btnRow->addWidget(btnCancel);
    outerLayout->addLayout(btnRow);

    connect(btnTagOn,  &QPushButton::clicked, this, &MainWindow::onTagOn);
    connect(btnRevert, &QPushButton::clicked, this, &MainWindow::onRevert);
    connect(btnCancel, &QPushButton::clicked, this, &MainWindow::onCancel);

    setWindowTitle("NMAAHC mkvnote");

    if (!inputFile.isEmpty())
        loadFile(inputFile);
}

//loadFile
void MainWindow::loadFile(const QString &path)
{
    m_inputFile = path;
    m_tagOrder.clear();
    m_fields.clear();

    clearForm();

    m_dropHint->hide();
    m_scrollArea->show();

    setWindowTitle(QString("mkvnote — %1").arg(QFileInfo(path).fileName()));
    initLog();

    buildForm(m_formContainer, m_formLayout);
    populateForm(extractExistingTags());
}

//clearForm
void MainWindow::clearForm()
{
    while (m_formLayout->rowCount() > 0)
        m_formLayout->removeRow(0);
}

//onOpenFile
void MainWindow::onOpenFile()
{
    QString path = QFileDialog::getOpenFileName(
        this, "Open MKV File", QDir::homePath(),
        "Matroska Files (*.mkv *.mka *.mks);;All Files (*)");
    if (!path.isEmpty())
        loadFile(path);
}

//drag-n-drop
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) return;
    QString path = urls.first().toLocalFile();
    if (path.endsWith(".mkv", Qt::CaseInsensitive) ||
        path.endsWith(".mka", Qt::CaseInsensitive) ||
        path.endsWith(".mks", Qt::CaseInsensitive))
        loadFile(path);
}

//buildForm
void MainWindow::buildForm(QWidget * /*container*/, QFormLayout *layout)
{
    QMap<QString, QString> existing = extractExistingTags();

    auto addRow = [&](const QString &tagName, bool readOnly) {
        m_tagOrder.append(tagName);

        bool isNmaahc = NMAAHC_TAG_SET.contains(tagName) || RO_TAGS.contains(tagName)
                        || tagName == "ATTACHMENTS";
        QString labelText = isNmaahc ? tagName : tagName + "*";
        if (readOnly) labelText += " [RO]";

        QLabel *lbl = new QLabel(labelText);
        lbl->setMinimumWidth(200);
        lbl->setStyleSheet("color: white;");

        if (readOnly) {
            QLineEdit *le = new QLineEdit;
            le->setReadOnly(true);
            le->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            le->setStyleSheet("background-color: #3a3a3a; color: #aaa; border: 1px solid #555;");
            layout->addRow(lbl, le);
            m_fields[tagName] = le;
        } else if (MULTILINE_TAGS.contains(tagName)) {
            QTextEdit *te = new QTextEdit;
            int h = FIELD_HEIGHTS.value(tagName, 90);
            te->setFixedHeight(h);
            te->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            te->setStyleSheet("background-color: #3a3a3a; color: white; border: 1px solid #555;");
            layout->addRow(lbl, te);
            m_fields[tagName] = te;
        } else {
            QLineEdit *le = new QLineEdit;
            le->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            le->setStyleSheet("background-color: #3a3a3a; color: white; border: 1px solid #555;");
            layout->addRow(lbl, le);
            m_fields[tagName] = le;
        }
    };

    //Section: Technical / Read-Only
    layout->addRow(makeSectionHeader("Technical and Hashes"), new QWidget);
    for (const QString &tag : RO_TAGS)
        addRow(tag, true);
    addRow("ATTACHMENTS", true);

    //Section: NMAAHC tags already in the file
    QStringList embeddedNmaahc;
    for (const QString &tag : NMAAHC_TAG_SET)
        if (existing.contains(tag)) embeddedNmaahc.append(tag);

    if (!embeddedNmaahc.isEmpty()) {
        layout->addRow(makeSectionHeader("NMAAHC Tags"), new QWidget);
        for (const QString &tag : embeddedNmaahc)
            addRow(tag, false);
    }

    //Section: NMAAHC tags not yet in the file
    QStringList emptyNmaahc;
    for (const QString &tag : NMAAHC_TAG_SET)
        if (!existing.contains(tag)) emptyNmaahc.append(tag);

    if (!emptyNmaahc.isEmpty()) {
        layout->addRow(makeSectionHeader("Empty NMAAHC Tags"), new QWidget);
        for (const QString &tag : emptyNmaahc)
            addRow(tag, false);
    }

    //Section: Extra tags in the file not in the NMAAHC set
    QStringList extraTags;
    for (const QString &tag : existing.keys())
        if (!NMAAHC_TAG_SET.contains(tag) && !RO_TAGS.contains(tag))
            extraTags.append(tag);
    extraTags.sort();

    if (!extraTags.isEmpty()) {
        layout->addRow(makeSectionHeader("Extra Existing Tags"), new QWidget);
        for (const QString &tag : extraTags)
            addRow(tag, false);
    }
}

//extractExistingTags 
QMap<QString, QString> MainWindow::extractExistingTags()
{
    QMap<QString, QString> tags;
    if (m_inputFile.isEmpty()) return tags;

    QProcess proc;
    proc.start("mkvextract", {"tags", m_inputFile});
    proc.waitForFinished(30000);
    QByteArray xmlData = proc.readAllStandardOutput();

    if (xmlData.isEmpty()) return tags;

    QXmlStreamReader xml(xmlData);
    while (!xml.atEnd()) {
        xml.readNext();
        if (!xml.isStartElement() || xml.name() != QLatin1String("Tag"))
            continue;

        QList<QPair<QString,QString>> simples;
        bool targetsFound = false;
        bool targetsHasChildren = false;

        while (!xml.atEnd()) {
            xml.readNext();
            if (xml.isEndElement() && xml.name() == QLatin1String("Tag"))
                break;
            if (!xml.isStartElement()) continue;

            if (xml.name() == QLatin1String("Targets")) {
                targetsFound = true;
                while (!xml.atEnd()) {
                    xml.readNext();
                    if (xml.isEndElement() && xml.name() == QLatin1String("Targets"))
                        break;
                    if (xml.isStartElement())
                        targetsHasChildren = true;
                }
            } else if (xml.name() == QLatin1String("Simple")) {
                QString name, value;
                while (!xml.atEnd()) {
                    xml.readNext();
                    if (xml.isEndElement() && xml.name() == QLatin1String("Simple"))
                        break;
                    if (xml.isStartElement()) {
                        if (xml.name() == QLatin1String("Name"))
                            name = xml.readElementText();
                        else if (xml.name() == QLatin1String("String"))
                            value = xml.readElementText();
                    }
                }
                if (!name.isEmpty())
                    simples.append({name, value});
            }
        }

        bool isGlobal = !targetsFound || !targetsHasChildren;
        if (isGlobal) {
            for (auto &p : simples)
                tags[p.first] = p.second;
        }
    }
    return tags;
}

//getAttachments 
QString MainWindow::getAttachments()
{
    QProcess proc;
    proc.start("mediainfo", {"--Output=General;%Attachments%", m_inputFile});
    proc.waitForFinished(10000);
    QString result = proc.readAllStandardOutput().trimmed();
    if (result.isEmpty() || result == "N/A") return "none";
    return result.replace(" / ", " ; ");
}

//onTagOn
void MainWindow::onTagOn()
{
    if (m_inputFile.isEmpty()) {
        QMessageBox::warning(this, "mkvnote", "No file loaded. Please open an MKV file first.");
        return;
    }

    log("INFO", "Processing tags for XML generation");

    QMap<QString, QString> tagValues = extractExistingTags();

    for (const QString &tag : m_tagOrder) {
        if (tag == "ATTACHMENTS") continue;
        if (RO_TAGS.contains(tag)) continue;

        QWidget *w = m_fields.value(tag);
        if (!w) continue;

        QString value;
        if (auto *te = qobject_cast<QTextEdit*>(w))
            value = te->toPlainText();
        else if (auto *le = qobject_cast<QLineEdit*>(w))
            value = le->text();

        if (!value.trimmed().isEmpty())
            tagValues[tag] = value;
        else
            tagValues.remove(tag);
    }

    if (!writeTagsToMkv(tagValues)) return;

    QMessageBox::information(this, "mkvnote",
        QString("Tags written successfully to:\n%1").arg(m_inputFile));
    close();
}

//onCancel
void MainWindow::onCancel()
{
    log("INFO", "User cancelled operation");
    close();
}

//populateForm
void MainWindow::populateForm(const QMap<QString, QString> &tags)
{
    for (const QString &tag : m_tagOrder) {
        QWidget *w = m_fields.value(tag);
        if (!w) continue;

        if (tag == "ATTACHMENTS") {
            if (auto *le = qobject_cast<QLineEdit*>(w))
                le->setText(getAttachments());
        } else if (RO_TAGS.contains(tag)) {
            if (auto *le = qobject_cast<QLineEdit*>(w))
                le->setText(tags.value(tag));
        } else if (auto *te = qobject_cast<QTextEdit*>(w)) {
            te->setPlainText(tags.value(tag));
        } else if (auto *le = qobject_cast<QLineEdit*>(w)) {
            le->setText(tags.value(tag));
        }
    }
}

//onRevert
void MainWindow::onRevert()
{
    if (m_inputFile.isEmpty()) return;

    auto reply = QMessageBox::question(this, "Revert",
        "Discard all changes and reload tags from file?",
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    log("INFO", "User reverted to saved tags");
    populateForm(extractExistingTags());
}

//writeTagsToMkv 
bool MainWindow::writeTagsToMkv(const QMap<QString, QString> &tags)
{
    QTemporaryFile tmpXml;
    tmpXml.setFileTemplate("/tmp/mkvnote_XXXXXX");
    tmpXml.setAutoRemove(false);
    if (!tmpXml.open()) {
        QMessageBox::critical(this, "mkvnote",
            QString("Could not create temp XML file: %1").arg(tmpXml.errorString()));
        return false;
    }

    QString xmlContent;
    QXmlStreamWriter xml(&xmlContent);
    xml.setAutoFormatting(true);
    xml.writeStartElement("Tags");
    xml.writeStartElement("Tag");
    xml.writeEmptyElement("Targets");
    for (auto it = tags.constBegin(); it != tags.constEnd(); ++it) {
        xml.writeStartElement("Simple");
        xml.writeTextElement("Name", it.key());
        xml.writeTextElement("String", it.value());
        xml.writeEndElement();
    }
    xml.writeEndElement(); // Tag
    xml.writeEndElement(); // Tags

    QTextStream out(&tmpXml);
    out << xmlContent;
    tmpXml.close();

    QString xmlPath = tmpXml.fileName();
    logXml(xmlContent);
    log("INFO", QString("Writing tags to MKV: %1").arg(m_inputFile));
    log("INFO", QString("XML temp file: %1").arg(xmlPath));

    QProcess proc;
    proc.start("mkvpropedit", {"--tags", QString("global:%1").arg(xmlPath), m_inputFile});
    proc.waitForFinished(60000);
    int exitCode = proc.exitCode();
    QString output = proc.readAllStandardOutput() + proc.readAllStandardError();

    log("DEBUG", QString("mkvpropedit exit: %1").arg(exitCode));
    log("DEBUG", QString("mkvpropedit output: %1").arg(output));

    QFile::remove(xmlPath);

    if (exitCode != 0) {
        log("ERROR", QString("mkvpropedit failed: %1").arg(output));
        QMessageBox::critical(this, "mkvnote",
            QString("Error writing tags:\n%1").arg(output));
        return false;
    }

    log("INFO", "Tags written successfully");
    return true;
}

//initLog
void MainWindow::initLog()
{
    QFileInfo fi(m_inputFile);
    m_logFile = fi.dir().filePath(fi.completeBaseName() + "_mkvnote_tags.log");
    QFile f(m_logFile);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream s(&f);
        s << "# mkvnote log - " << QDateTime::currentDateTimeUtc().toString(Qt::ISODate) << "\n";
        s << "# Version: 1.0.0\n";
        s << "# Input: " << m_inputFile << "\n";
        s << "# ========================================\n";
    }
}

//log
void MainWindow::log(const QString &level, const QString &message)
{
    if (m_logFile.isEmpty()) return;
    QFile f(m_logFile);
    if (f.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream s(&f);
        s << "[" << QTime::currentTime().toString("HH:mm:ss") << "] "
          << "[" << level << "] " << message << "\n";
    }
}

//logXml
void MainWindow::logXml(const QString &xmlContent)
{
    if (m_logFile.isEmpty()) return;
    QFile f(m_logFile);
    if (f.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream s(&f);
        s << "\n# ======== XML CONTENT ========\n";
        s << xmlContent;
        s << "\n# ======== END XML ========\n";
    }
}

//makeSectionHeader 
QLabel *MainWindow::makeSectionHeader(const QString &text)
{
    QLabel *lbl = new QLabel(QString("<b><span style='color:#C97FD4'>%1</span></b>").arg(text));
    lbl->setTextFormat(Qt::RichText);
    lbl->setObjectName("sectionHeader");
    lbl->setContentsMargins(0, 12, 0, 4);
    return lbl;
}
