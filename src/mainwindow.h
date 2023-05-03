#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QSystemTrayIcon>
#include <QTimer>
#include "aboutdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	struct AttackMode{
		enum{
			restore,
			wordlist,
			wordlist_rules,
			brutforce,
		};
	};
	struct AttackSpeed{
		enum{
			Low		= 1,
			Economic,
			High,
			Insane,
		};
	};
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();
	void init();
private slots:
	void slot_authenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);
//	void slot_authenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator);
	void slot_timerUpdate();
	void slot_updateProgress(const qint64 bytesSent, const qint64 bytesTotal);
private:
	Ui::MainWindow *ui;
	AboutDialog* m_pAboutWindow;
	QNetworkAccessManager* m_pNetworkManager;
	QNetworkReply *m_reply;
	QTimer* m_pTimer;
	QTimer* m_pTimerProgress;
	bool m_running;
	bool m_infoProcess;
	float m_progress;
	uint8_t m_attackMode;
	QByteArray m_inputBuff;
	uint64_t m_uploadBytes;
	uint64_t m_prewUploadBytes;
	uint64_t m_speed;

	void updateUI();
};
#endif // MAINWINDOW_H
