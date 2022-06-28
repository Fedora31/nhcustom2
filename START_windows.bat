@echo off

set INPUT="./input/"
set OUTPUT="./output/"

echo -- nhcustom2 --


::replace the output directory if it exists

if exist %OUTPUT% rmdir /s /q  %OUTPUT%
mkdir %OUTPUT%

echo reading the configuration file...


::store the output of the program

nhcustom2.exe > paths.txt

echo copying files...


::send each line of the file to the process label

for /f  %%a in (paths.txt) do call :process %%a

del paths.txt
echo done.
goto :eof


:process


::%~nx1 means the filename+extention of arg 1
::and a bunch of flags to make robocopy silent

::is there really no way to get the parent dir?!!?
robocopy "%INPUT%%1/../" "%OUTPUT%%1/../" "%~nx1"  /njh /njs /ndl /nc /ns

goto :eof


:eof

::writing this batch script was the single most %&*# thing I ever did
