#!/bin/bash

IsGameOver=false;
WinnerSymbol="";

IsPlayerServer=false;
IsPlayerWinner=false;

Blank=" ";
XSymbol="X";
OSymbol="O";

DefaultColor="0";
NotificationColor="1";
XColor="2";
OColor="4";
BordersColor="7";

GameField=();

#методы для отрисовки всякого
function PrintHelp {
	echo "Tic-Tac-Toe game";
	echo "Parameters:";
	echo "            -c to start game for client";
	echo "            -s to start game for server";
	echo "You need to start server to start playing";
}
function PrintForServer {
	SetColor $NotificationColor;
	echo "                 ...You are now playing as Server...";
	SetColor $DefaultColor;
}
function PrintForClient {
	SetColor $NotificationColor;
	echo "                 ...You are now playing as Client...";
	SetColor $DefaultColor;
}
function PrintCommonRules {
	echo "You need to input two numbers to make a move.";
	echo "Do it like this: '1 2' (x and y coordinates).";
	echo "Have a lucky game!";
}
function PrintGameField {
	SetCursor 5 0;
	SetColor $BordersColor;
	echo "---------------";
	for i in `seq 0 2`
	do
		for j in `seq 0 2`
		do
			GetSymbol $i $j;
			displayedSymbol=$Blank;

			echo -n " | ";
			if [[ $currentSymbol == $XSymbol ]]
			then
				displayedSymbol=$XSymbol;
				SetColor $XColor;
			elif [[ $currentSymbol == $OSymbol ]]
			then
				displayedSymbol=$OSymbol;
				SetColor $OColor;
			fi
			echo -n "$displayedSymbol";
			SetColor $BordersColor;
		done
		echo " |";
		echo "---------------";
	done
	SetColor $DefaultColor;
}

function PrepareForInput {
	echo -n $1;
	tput sc;
	echo "                        ";
	tput rc;
}
function PrepareForPlayerTurn {
	ShowCursor;
	SetCursor 13 0;

	echo "Waiting for your move...                                             "
	if [[ "$IsInputInvalid" == "true" ]]
	then
		PrepareForInput "Wrong coodinates, try again:                          ";
	elif [[ "$IsPositionTaken" == "true" ]]
	then
		PrepareForInput "This position is already taken, try again:            ";
	else
		PrepareForInput "Make your move:                                       ";
	fi
}
function PrepareForOpponentTurn {
	HideCursor;
	SetCursor 13 0;
		
	echo "Waiting for opponent move...                                         ";
	echo "                                                                     ";
}
function PrintWinner {
	if [[ $WinnerSym == $PlayerSymbol ]]
	then
		echo "Congratulations! You are the winner!                             ";
	elif [[ $WinnerSym == $OpponentSymbol ]]
	then
		echo "Sorry, you lost.                                                 ";
	else
		echo "This is a draw.                                                  ";
	fi
	echo "                                                                     ";
	echo "                                                                     ";
}

#методы для работы с полем по координатам (геттер/сеттер)
function GetSymbol {
	currentSymbol="${GameField[3*$1+$2]}";
}
function SetSymbol {
	GameField[3*$1+$2]=$3;
}

#валидаторы
function ValidateParameters {
	if [[ "$1" -ne "1" ]]
	then
		echo "Error: you entered wrong parameters.";
		PrintHelp;
		exit 0;
	fi
}
function ValidateCoordinate {
	if [[ $1 == "" || $1 -lt 1 || $1 -gt 3 ]]
	then
		IsInputInvalid=true;
		return;
	fi
}
function ValidateCoordinates {
	IsInputInvalid=false;
	ValidateCoordinate $1;
	ValidateCoordinate $2;
}
function ValidatePosition {
	IsPositionTaken=false;
	let x=$1-1;
	let y=$2-1;
	GetSymbol $x $y;
	if [[ $currentSymbol == $XSymbol || $currentSymbol == $OSymbol ]]
	then
		IsPositionTaken=true;
	fi
}

#методы для отрисовки ходов своих или противника
function MakeMove {
	let x=$1-1;
	let y=$2-1;
	SetSymbol $x $y $PlayerSymbol;
	HasNextTurn=false;
	PrintGameField;
}
function DisplayOpponentMove {
	let x=$1-1;
	let y=$2-1;
	SetSymbol $x $y $OpponentSymbol;
	HasNextTurn=true;
	PrintGameField;
}

#методы для проверки окончания игры
function CheckPositions {
	combination=${GameField[$1]}${GameField[$2]}${GameField[$3]};
	if [[ $combination == "XXX" || $combination == "OOO" ]]
	then
		WinnerSym=${GameField[$1]};
		IsGameOver=true;
		return;
	fi
}
function CheckIfAllFieldsAreTaken {
	let empty=true;
	for i in `seq 0 8`
	do
		if [[ ${GameField[$i]} == $Blank ]]
		then
			return;
		fi
	done
	
	IsGameOver=true;
	WinnerSym=$Blank;
}
function CheckHorizontalCombinations {
	CheckPositions 0 1 2;
	CheckPositions 3 4 5;
	CheckPositions 6 7 8;
}
function CheckVerticalCombinations {
	CheckPositions 0 3 6;
	CheckPositions 1 4 7;
	CheckPositions 2 5 8;
}
function CheckDiagonalCombinations {
	CheckPositions 0 4 8;
	CheckPositions 2 4 6;
}
function CheckIfGameIsOver {
	CheckHorizontalCombinations;
	CheckVerticalCombinations;
	CheckDiagonalCombinations;
	CheckIfAllFieldsAreTaken;
}
function CheckForPlayerExit {
	if [[ "$1" == "exit" ]]
	then
		echo "You left the game";
		echo -n $1>$Output;
		ShowCursor;
		exit 0;
	fi
}
function CheckForOpponentExit {
	if [[ "$1" == "exit" ]]
	then
		echo "Your opponent left the game";
		ShowCursor;
		exit 0;
	fi
}

#инициализаторы
function InitGameField {
	for i in `seq 0 8`
	do
		GameField[$i]=$Blank;
	done
}
function InitPlayerType {
	if [[ "$1" == "-h" ]]
	then
		PrintHelp;
		exit 1;
	elif [[ "$1" == "-s" ]]
	then
		IsPlayerServer=true;
	elif [[ "$1" != "-c" ]]
	then
		echo "Error: unsupported parameter '$1'";
		PrintHelp;
		exit 1;
	fi
}
function InitPipesIfNeeded {
	if [[ ! -p $GamePipe1 ]]
	then 
		mknod $GamePipe1 p;
	fi

	if [[ ! -p $GamePipe2 ]]
	then
		mknod $GamePipe2 p;
	fi
}
function ClearPipes {
	if [[ "$IsPlayerServer" == "true" ]]
	then
		if [[ -p $GamePipe1 ]]
		then
			rm $GamePipe1;
		fi
		if [[ -p $GamePipe2 ]]
		then
			rm $GamePipe2;
		fi
	fi
}

#конфигурации для сервера/клиента
function ConfigureGameForServer {
	Input=$GamePipe1;
	Output=$GamePipe2;

	HasNextTurn=false;

	PlayerSymbol=$XSymbol;
	OpponentSymbol=$OSymbol;

	PlayerColor=$XColor;
	OpponentColor=$OColor;
}
function ConfigureGameForClient {
	Input=$GamePipe2;
	Output=$GamePipe1;

	HasNextTurn=true;

	PlayerSymbol=$OSymbol;
	OpponentSymbol=$XSymbol;

	PlayerColor=$OColor;
	OpponentColor=$XColor;
}

#чтобы не начать игру без сервера
function PingServer {
	if [[ ! -p $GamePipe1 || ! -p $GamePipe2 ]]
	then
		echo "Error: server is not started.";
		exit 1;
	fi
}

#совсем базовые обертки, а то голыми на них смотреть больно
function ShowCursor {
	tput cnorm;
}
function HideCursor {
	tput civis;
}
function SetCursor {
	tput cup $1 $2;
}
function SetColor {
	tput setf $1;
}

#-----------------------------------MAIN PROGRAM-------------------------------------

ValidateParameters $#;
InitPlayerType $1;

GamePipe1=/tmp/1
GamePipe2=/tmp/2

if [[ "$IsPlayerServer" == "true" ]]
then
	InitPipesIfNeeded;
	ConfigureGameForServer;
else
	PingServer;
	ConfigureGameForClient;
fi

clear;

PrintCommonRules;

if [[ "$IsPlayerServer" == "true" ]]
then
	PrintForServer;
else
	PrintForClient;
fi

InitGameField;
PrintGameField;

while true
do
	PingServer;
	if [[ "$HasNextTurn" == "true" ]]
	then
		PrepareForPlayerTurn;

		read xCoor yCoor;
		
		CheckForPlayerExit $xCoor;

		ValidateCoordinates $xCoor $yCoor;

		if [[ "$IsInputInvalid" == "true" ]]
		then
			continue;
		fi

		ValidatePosition $xCoor $yCoor;
		if [[ "$IsPositionTaken" == "true" ]]
		then
			continue;
		fi
		
		MakeMove $xCoor $yCoor;

		echo -n $xCoor $yCoor>$Output;

		CheckIfGameIsOver;

		if [[ "$IsGameOver" == "true" ]]
		then
			break;
		fi

	else
		PrepareForOpponentTurn;
		
		read xCoor yCoor<$Input;
		
		CheckForOpponentExit $xCoor;
		
		ValidateCoordinates $xCoor $yCoor;

		if [[ "$IsInputInvalid" == "true" ]]
		then
			continue;
		fi
		
		DisplayOpponentMove $xCoor $yCoor;

		CheckIfGameIsOver;

		if [[ "$IsGameOver" == "true" ]]
		then
			break;
		fi
	fi
done

PrintWinner;
ClearPipes;
ShowCursor;

exit 0;	