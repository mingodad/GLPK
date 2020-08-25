param p symbolic := "dad";
check length(p) == 3;

#{
#call trunc(33.4);
#}

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
    #break;
}

#break;

for{i in 1..10} {
    if i < 3 then continue;
    if i == 8 then break;
    display i;
}


for{i in 1..4} {
    printf "now we are at %d\n", i;
    if i mod 2 = 0 then { #assert when missing "= 0"
        param lp := i+10;
        set ls := {1..i+1};
        printf "nested if lp = %d\n", lp;
        display ls;
    }
    param lp := i+20;
    set ls := {1..i+2};
    printf "nested for lp = %d\n", lp;
    display ls;
}
