// =================================================================
// Logtalk - Object oriented extension to Prolog
// Release 2.28.2
//
// Copyright (c) 1998-2006 Paulo Moura.  All Rights Reserved.
// =================================================================

if (ScriptEngineMajorVersion() < 5 || ScriptEngineMajorVersion() == 5 && ScriptEngineMinorVersion() < 6) {
	WScript.Echo('Error! WSH 5.6 or later version needed for running this script.');
	WScript.Quit(1);
}

if (WScript.Arguments.Unnamed.Length > 0) {
	usage_help();
	WScript.Quit(0);
}

WScript.Echo('');
WScript.Echo('Creating a shortcut named "Logtalk - B-Prolog" for running');
WScript.Echo('Logtalk with B-Prolog 6.9 ...');
WScript.Echo('');

var WshShell = new ActiveXObject("WScript.Shell");

var prolog_path = WshShell.RegRead("HKLM\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment\\BPDIR") + "\\bp.bat";

var FSObject = new ActiveXObject("Scripting.FileSystemObject");

var WshSystemEnv = WshShell.Environment("SYSTEM");
var WshUserEnv = WshShell.Environment("USER");
var logtalk_home;

if (WshSystemEnv.Item("LOGTALKHOME"))
	logtalk_home = WshSystemEnv.Item("LOGTALKHOME");
else if (WshUserEnv.Item("LOGTALKHOME"))
	logtalk_home = WshUserEnv.Item("LOGTALKHOME")
else {
	WScript.Echo("Error! The environment variable LOGTALKHOME must be defined first!");
	usage_help();
	WScript.Quit(1);
}

if (!FSObject.FolderExists(logtalk_home)) {
	WScript.Echo("The environment variable LOGTALKHOME points to a non-existing directory!");
	WScript.Echo("Its current value is: %LOGTALKHOME%");
	WScript.Echo("The variable must be set to your Logtalk installation directory!");
	WScript.Echo("");
	usage_help();
	WScript.Quit(1);
}

logtalk_home = logtalk_home.replace(/\\/g, "\\\\");

if (!FSObject.FolderExists(logtalk_home + "\\bin"))
	FSObject.CreateFolder(logtalk_home + "\\bin");

WshShell.Run("bp -g \"set_prolog_flag(redefined, off), compile('" + logtalk_home + "\\\\compiler\\\\logtalk.pl')\"", 1, true);
if (FSObject.FileExists(logtalk_home + "\\bin\\logtalk.pl.out"))
   FSObject.DeleteFile(logtalk_home + "\\bin\\logtalk.pl.out");
FSObject.MoveFile(logtalk_home + "\\compiler\\logtalk.pl.out", logtalk_home + "\\bin\\");

var f = FSObject.CreateTextFile(logtalk_home + "\\bin\\logtalk_bp.pl", true);

f.WriteLine(":- set_prolog_flag(redefined, off).");
f.WriteLine(":- compile('%LOGTALKUSER%\\\\configs\\\\b.config').");
f.WriteLine(":- load('%LOGTALKUSER%\\\\configs\\\\b.config').");
f.WriteLine(":- load('%LOGTALKHOME%\\\\bin\\\\logtalk.pl').");
f.WriteLine(":- compile('%LOGTALKUSER%\\\\libpaths\\\\libpaths.pl').");
f.WriteLine(":- load('%LOGTALKUSER%\\\\libpaths\\\\libpaths.pl').");
f.Close();

var ProgramsPath = WshShell.SpecialFolders("AllUsersPrograms");

if (!FSObject.FolderExists(ProgramsPath + "\\Logtalk")) 
	FSObject.CreateFolder(ProgramsPath + "\\Logtalk");

var link = WshShell.CreateShortcut(ProgramsPath + "\\Logtalk\\Logtalk - B-Prolog.lnk");
link.Arguments = "-g \"consult('" + logtalk_home + "\\\\bin\\\\logtalk_bp.pl'), $bp_top_level\"";
link.Description = 'Runs Logtalk with B-Prolog';
link.IconLocation = 'app.exe,1';
link.TargetPath = prolog_path;
link.WindowStyle = 1;
link.WorkingDirectory = logtalk_home;
link.Save();

WScript.Echo('Done. The "Logtalk - B-Prolog" shortcut was been added to the');
WScript.Echo('Start Menu Programs. Make sure that the environment variables');
WScript.Echo('LOGTALKHOME and LOGTALKUSER are defined for all users wishing');
WScript.Echo('to use the shortcut.');
WScript.Echo('');
WScript.Echo('Users must run the batch script "cplgtdirs" before using the');
WScript.Echo('"Logtalk - B-Prolog" shortcut.');
WScript.Echo('');

WScript.Quit(0);

function usage_help() {
	WScript.Echo('');
	WScript.Echo('This script creates a shortcut named "Logtalk - B-Prolog" for');
	WScript.Echo('running Logtalk with B-Prolog. The script must be run by a user');
	WScript.Echo('with administrative rights. The LOGTALKHOME environment variable');
	WScript.Echo('must be defined before running this script.');
	WScript.Echo('');
	WScript.Echo('Usage:');
	WScript.Echo('  ' + WScript.ScriptName + ' help');
	WScript.Echo('  ' + WScript.ScriptName);
	WScript.Echo('');
}
  