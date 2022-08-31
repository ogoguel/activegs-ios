// ActiveGSApp.cpp : Définit les comportements de classe pour l'application.
//

#include "../Common.win32/stdafx.h"
#include "ActiveGSApp.h"
#include "MainFrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CActiveGSAppAp
BEGIN_MESSAGE_MAP(CActiveGSAppApp, CWinApp)
//	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
END_MESSAGE_MAP()


// construction CActiveGSAppApp

CActiveGSAppApp::CActiveGSAppApp()
{
	// TODO : ajoutez ici du code de construction,
	// Placez toutes les initialisations significatives dans InitInstance
}


// Seul et unique objet CActiveGSAppApp

CActiveGSAppApp theApp;

// initialisation CActiveGSAppApp

BOOL CActiveGSAppApp::InitInstance()
{
	// InitCommonControls() est requis sur Windows XP si le manifeste de l'application
	// spécifie l'utilisation de ComCtl32.dll version 6 ou ultérieure pour activer
	// les styles visuels.  Dans le cas contraire, la création de fenêtres échouera.
	InitCommonControls();

	CWinApp::InitInstance();

	// Initialiser les bibliothèques OLE
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// Initialisation standard
	// Si vous n'utilisez pas ces fonctionnalités et que vous souhaitez réduire la taille
	// de votre exécutable final, vous devez supprimer ci-dessous
	// les routines d'initialisation spécifiques dont vous n'avez pas besoin.
	// Changez la clé de Registre sous laquelle nos paramètres sont enregistrés.
	// TODO : Modifiez cette chaîne avec des informations appropriées,
	// telles que le nom de votre société ou organisation
	SetRegistryKey(_T("Applications locales générées par AppWizard"));
	// Pour créer la fenêtre principale, ce code crée un nouvel objet fenêtre frame
	// qu'il définit ensuite en tant qu'objet fenêtre principale de l'application
	
	int argc = __argc;
	char** argv = __argv;


	CMainFrame* pFrame = new CMainFrame(argc,argv);
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;
	
	// MEts à jour le nom de la fenetre

	// La seule fenêtre a été initialisée et peut donc être affichée et mise à jour
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
	// appelle DragAcceptFiles uniquement s'il y a un suffixe
	//  Dans une application SDI, cet appel doit avoir lieu juste après ProcessShellCommand
	return TRUE;
}

