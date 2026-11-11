#pragma once

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileDevice>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QString>

namespace biofeedback::runtime {

inline QString appConfigDirectory()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (path.isEmpty()) path = QDir::home().filePath(".config/biofeedback");
    QDir().mkpath(path);
    return path;
}

inline QString appDataDirectory()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (path.isEmpty()) path = QDir::home().filePath(".local/share/biofeedback");
    QDir().mkpath(path);
    return path;
}

inline QString installedConfigFile(const QString& fileName)
{
    const QString relative = QStringLiteral("biofeedback/config/") + fileName;
    const QString located = QStandardPaths::locate(QStandardPaths::GenericDataLocation, relative);
    if (!located.isEmpty()) return located;

    const QString besideBinary = QDir(QCoreApplication::applicationDirPath())
        .absoluteFilePath(QStringLiteral("../share/biofeedback/config/") + fileName);
    if (QFile::exists(besideBinary)) return QDir::cleanPath(besideBinary);

    const QString sourceTree = QDir::current().filePath(QStringLiteral("config/") + fileName);
    if (QFile::exists(sourceTree)) return sourceTree;
    return QString();
}

inline QString ensureUserConfigFile(const QString& fileName)
{
    const QString destination = QDir(appConfigDirectory()).filePath(fileName);
    if (QFile::exists(destination)) return destination;

    const QString source = installedConfigFile(fileName);
    if (!source.isEmpty()) {
        if (!QFile::copy(source, destination)) return QString();
    } else if (fileName == QStringLiteral("users.json")) {
        QFile emptyUsers(destination);
        if (!emptyUsers.open(QIODevice::WriteOnly | QIODevice::Text)) return QString();
        emptyUsers.write("{\n  \"users\": []\n}\n");
        emptyUsers.close();
    } else {
        return QString();
    }

    // Account names and password hashes are private to the current OS user.
    QFile::setPermissions(destination, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    return destination;
}

inline QString patientDirectory(const QString& patientId)
{
    const QString root = QDir(appDataDirectory()).filePath(QStringLiteral("patients"));
    QDir().mkpath(root);
    if (patientId.trimmed().isEmpty()) {
        const QString unassigned = QDir(root).filePath(QStringLiteral("unassigned"));
        QDir().mkpath(unassigned);
        return unassigned;
    }

    // Do not expose PESEL or another direct identifier in filesystem paths.
    const QString pseudonymousId = QString::fromLatin1(
        QCryptographicHash::hash(patientId.toUtf8(), QCryptographicHash::Sha256).toHex());
    const QString path = QDir(root).filePath(pseudonymousId);

    // Migrate legacy directories that used a sanitized patient identifier.
    QString legacyId = patientId;
    legacyId.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9_-]")), QStringLiteral("_"));
    const QString legacyPath = QDir(root).filePath(legacyId);
    if (legacyPath != path && QDir(legacyPath).exists() && !QDir(path).exists())
        QDir().rename(legacyPath, path);

    QDir().mkpath(path);
    return path;
}

} // namespace biofeedback::runtime
