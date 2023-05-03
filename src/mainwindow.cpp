#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "global.h"
#include "ui_authenticationdialog.h"
#include "myfunctions.h"



#include <QAuthenticator>
#include <QFileDialog>
#include <QHostAddress>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QMessageBox>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QSettings>
#include <QSslError>
#include <QStyle>
#ifdef QT_DEBUG
	#include <QDebug>
#endif

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);



	m_pTimer				= new QTimer( this );
	m_pTimerProgress		= new QTimer( this );
	m_running				= false;
	m_infoProcess			= false;
	m_progress				= 0;
	m_attackMode			= MainWindow::AttackMode::restore;
	m_pAboutWindow			= new AboutDialog( this );
	m_pNetworkManager		= new QNetworkAccessManager( this );




	ui->fileLE->setText( app::conf.file );
	ui->hostLE->setText( app::conf.host );
	ui->startStopB->setText( "" );

	m_pTimerProgress->setInterval( 1000 );
	m_pTimer->setInterval( 2500 );
	m_pTimer->start();
	ui->progressBar->setValue( 0 );
	ui->fsProgressBar->setValue( 0 );


	updateUI();


	connect( m_pTimerProgress, &QTimer::timeout, this, [ this ](){
		m_speed = m_uploadBytes - m_prewUploadBytes;
		ui->speedL->setText( QString( "%1/s" ).arg( mf::getSize( m_speed ) ) );
	} );
	connect( m_pTimer, &QTimer::timeout, this, &MainWindow::slot_timerUpdate );
	connect( ui->actionAbout, &QAction::triggered, m_pAboutWindow, &AboutDialog::show );
	connect( ui->fileSelectB, &QPushButton::clicked, this, [this](){
		QString fileName = QFileDialog::getOpenFileName( this, tr( "Open hash file" ), ( ui->fileLE->text().isEmpty() ) ? QDir::homePath() : ui->fileLE->text(), tr( "All Files (*)" ));
		if( fileName != "" ){
			ui->fileLE->setText( fileName );
			app::conf.file = fileName;
		}
	} );
	connect( ui->startStopB, &QPushButton::clicked, this, [this](){
		if( !m_running ){

			auto filePath = ui->fileLE->text();
			if( !mf::checkFile( filePath ) ) return;

			QByteArray data;
			QFileInfo info( filePath );

			QUrl url = QUrl( "http://" + ui->hostLE->text() + "/update" );
			QByteArray boundary;
			boundary.append( "---------------------------723690991551375881941828858" );

			data.append( "--" ); data.append( boundary ); data.append( "\r\n" );
			data.append( "Content-Disposition: form-data; name=\"sdf\"\r\n\r\n" );
			data.append( ui->keyLE->text().toUtf8() );
			data.append( "\r\n" );

			QFile file;
			file.setFileName( filePath );
			if( file.open( QIODevice::ReadOnly ) ){
				data.append( "--" ); data.append( boundary ); data.append( "\r\n" );

				if( ui->typeFileRB->isChecked() ){
					data.append( "Content-Disposition: form-data; name=\"file\" filename=\"" );
				}else if( ui->typeFirmwareRB->isChecked() ){
					data.append( "Content-Disposition: form-data; name=\"firmware\" filename=\"" );
				}else if( ui->typeFilesystemRB->isChecked() ){
					data.append( "Content-Disposition: form-data; name=\"filesystem\" filename=\"" );
				}

				data.append( info.baseName().toUtf8() );
				data.append( "." );
				data.append( info.suffix().toUtf8() );
				data.append( "\"\r\n" );
				if( info.suffix().toLower() == "html" ){
					data.append( "Content-Type: text/html\r\n" );
				}else{
					data.append( "Content-Type: application/octet-stream\r\n" );
				}
				data.append( "\r\n" );
				while( !file.atEnd() ){
					data.append( file.read( 1024 ) );
				}
				file.close();
				data.append( "\r\n" );
			}

//			data.append( "--" ); data.append( boundary ); data.append( "\r\n" );
//			data.append( "Content-Disposition: form-data; name=\"cmd\"\r\n" );
//			data.append( "Update\r\n" );
			data.append( "--" ); data.append( boundary ); data.append( "--\r\n" );
			if( data.size() == 0 ) return;

//			QHttpMultiPart multiPart( QHttpMultiPart::FormDataType );
//			QHttpPart filePart;
////			filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/zip"));
//			filePart.setHeader( QNetworkRequest::ContentDispositionHeader, QVariant( "form-data; name=\"file\" filename=\"" + info.baseName() + "\"" ) );
//			filePart.setHeader( QNetworkRequest::ContentLengthHeader, data.size() );
//			filePart.setBody( data );
//			multiPart.append( filePart );

			QNetworkRequest request;
			request.setUrl( url );
//			request.setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
			request.setHeader( QNetworkRequest::ContentTypeHeader, "multipart/form-data; boundary=" + boundary );
			request.setHeader( QNetworkRequest::ContentLengthHeader, data.size() );
//			request.setHeader( QNetworkRequest::ContentLengthHeader, 4 );
//			auto reply = m_pNetworkManager->put( request, data );
//			m_pNetworkManager->post( request, "onoff=0" );

			m_speed = 0;
			m_uploadBytes = 0;
			m_prewUploadBytes = 0;
			m_pTimerProgress->start();
			if( m_reply != nullptr ) disconnect( m_reply, &QNetworkReply::uploadProgress, this, &MainWindow::slot_updateProgress );
			m_reply = m_pNetworkManager->post( request, data );
			connect( m_reply, &QNetworkReply::uploadProgress, this, &MainWindow::slot_updateProgress );

//			m_pNetworkManager->put( request, data );
//			connect( reply, &QNetworkReply::readyRead, this, [reply](){
//				qDebug()<<reply->errorString() << reply->readAll();
//			} );
//			connect( reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater );

			m_running = true;
			updateUI();
		}else{
			if( m_reply != nullptr ) m_reply->abort();
			m_running = false;
			updateUI();
		}
	} );

	connect( m_pNetworkManager, &QNetworkAccessManager::authenticationRequired, this, &MainWindow::slot_authenticationRequired );
//	connect( m_pNetworkManager, &QNetworkAccessManager::proxyAuthenticationRequired, this, &MainWindow::slot_authenticationRequired );
//	connect( m_pNetworkManager, &QNetworkAccessManager::sslErrors, this, [this](QNetworkReply *reply, const QList<QSslError> &errors){
//		QString errorString;
//		for (const QSslError &error : errors) {
//			if (!errorString.isEmpty())
//				errorString += '\n';
//			errorString += error.errorString();
//		}

//		if (QMessageBox::warning(this, tr("SSL Errors"),
//								 tr("One or more SSL errors has occurred:\n%1").arg(errorString),
//								 QMessageBox::Ignore | QMessageBox::Abort) == QMessageBox::Ignore) {
//			reply->ignoreSslErrors();
//		}
//	} );

	connect( m_pNetworkManager, &QNetworkAccessManager::finished, this, [this](QNetworkReply *rep){
		m_running = false;
		updateUI();
		QByteArray reply = rep->readAll();
		uint16_t code = rep->error();
		m_pTimerProgress->stop();
		m_reply = nullptr;

		if( m_infoProcess ){
			m_infoProcess = false;
			if( code == 0 ){
				auto json = QJsonDocument::fromJson( reply );
				auto map = json.toVariant().toMap();
				if( map.contains( "version" ) ){
					auto list = map.values( "version" ).at( 0 ).toList();
					uint8_t fv = list.at( 0 ).toUInt();
					uint8_t sv = list.at( 1 ).toUInt();
					uint16_t tv = list.at( 2 ).toUInt();
					QString version = QString( "%1.%2.%3" ).arg( fv ).arg( sv ).arg( tv );
					ui->versionL->setText( version );
					ui->statusL->setText( "Available" );
				}else{
					ui->statusL->setText( "Available OLD version" );
				}

				if( map.contains( "fs_total" ) && map.contains( "fs_used" ) ){
					float fs_total = map.values( "fs_total" ).at( 0 ).toFloat();
					float fs_used = map.values( "fs_used" ).at( 0 ).toFloat();
					uint8_t prz = fs_used / ( fs_total / 100.0 );
					ui->fsProgressBar->setValue( prz );
				}
			}
			return;
		}

		switch( code ){
			case 2:		ui->statusL->setText( tr( "Remote Host Closed" ) );			break;
			case 99:	ui->statusL->setText( tr( "Unknown error" ) );				break;
			case 204:	ui->statusL->setText( tr( "Authentication Required" ) );	break;
		}

		QString str(reply);
		QMessageBox::information( this, ui->statusL->text(), QString( "[%1]:%2" ).arg( code ).arg( str ), "ok" );
	} );
}

MainWindow::~MainWindow()
{
	app::saveSettings();
	delete ui;
}

void MainWindow::init()
{

}

void MainWindow::slot_authenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator)
{
	QDialog authenticationDialog;
	Ui::Dialog ui;
	ui.setupUi(&authenticationDialog);
	authenticationDialog.adjustSize();
	ui.siteDescription->setText(tr("%1 at %2").arg(authenticator->realm(), reply->url().host()));

	// Did the URL have information? Fill the UI.
	// This is only relevant if the URL-supplied credentials were wrong
	ui.userEdit->setText( "admin" );
	ui.passwordEdit->setText( "admin" );

	if (authenticationDialog.exec() == QDialog::Accepted) {
		authenticator->setUser(ui.userEdit->text());
		authenticator->setPassword(ui.passwordEdit->text());
	}
}

//void MainWindow::slot_authenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator)
//{

//}

void MainWindow::slot_timerUpdate()
{
	auto host = ui->hostLE->text();
	if( app::conf.host != host ){
		app::conf.host = host;
	}

	if( !m_infoProcess && !m_running ){
		m_infoProcess = true;
		m_running = true;
		QNetworkRequest request;
		request.setUrl( "http://" + host + "/sysinfo" );
		m_pNetworkManager->get( request );
	}
}

void MainWindow::slot_updateProgress(const qint64 bytesSent, const qint64 bytesTotal)
{
	float prz = (float)bytesSent / ( (float)bytesTotal / 100.0 );
	ui->progressBar->setValue( prz );
	m_uploadBytes = bytesSent;
	if( m_speed > 0 ){
		uint64_t r = bytesTotal - bytesSent;
		ui->etaL->setText( QString( "~%1 sec." ).arg( r / m_speed ) );
	}
}

void MainWindow::updateUI()
{
	if( m_running ){
		ui->statusL->setText( tr( "Running" ) );
		ui->startStopB->setIcon( this->style()->standardIcon( QStyle::SP_MediaPause ) );
	}else{
		ui->startStopB->setIcon( this->style()->standardIcon( QStyle::SP_MediaPlay ) );
	}
}

