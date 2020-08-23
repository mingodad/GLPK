param p symbolic := "dad";
check length(p) == 3;

if length(p) == 3 then display "true";
if length(p) == 5 then display "true";
if length(p) == 3 then display "true"; else display "false";
if length(p) == 5 then display "true"; else display "false";

if length(p) == 3 then
    display "true";

if length(p) == 5 then
    display "true";

if length(p) == 3 then
    display "true";
else
    display "false";

if length(p) == 5 then
    display "true";
else
    display "false";

if length(p) == 3 then {display "true";}
if length(p) == 5 then {display "true";}
if length(p) == 3 then {}
if length(p) == 5 then {}
if length(p) == 3 then {display "true";} else {display "false";}
if length(p) == 5 then {display "true";} else {display "false";}

if length(p) == 3 then
{
    display "true";
}

if length(p) == 5 then
{
    display "true";
}

if length(p) == 3 then
{
    display "true";
}
else
{
    display "false";
}

if length(p) == 5 then
{
    display "true";
}
else
{
    display "false";
}

for{i in 1..10} {
    if i < 3 then continue;
    if i == 8 then break;
    display i;
}