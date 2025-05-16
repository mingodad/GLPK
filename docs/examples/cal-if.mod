/* cal.mod - print an ASCII calendar of the given year */

/* Written in GNU MathProg by Andrew Makhorin <mao@gnu.org> */

param year, integer, >= 0001, <= 3999, default 2010;
param lm := 12; # last month
set months := 1..lm;

param first_day{m in months}, integer, >= 0, <= 6, :=
      time2str(str2time(year & "-" & m & "-01", "%Y-%m-%d"), "%w");

param days_in_month{m in months}, integer, >= 28, <= 31, :=
      (str2time(year + (if m < lm then 0 else 1) & "-" &
         (if m < lm then m+1 else 1) & "-01", "%Y-%m-%d") -
      str2time(year & "-" & m & "-01", "%Y-%m-%d")) / 86400;

param foo{m in months, k in 0..5, d in 0..6}, integer, :=
      7 * k + d + 1 - first_day[m];

param cal{m in months, k in 0..5, d in 0..6}, integer, :=
      if 1 <= foo[m,k,d] and foo[m,k,d] <= days_in_month[m] then
         foo[m,k,d];

printf "\n";
printf "%33s%04d\n", "", year;
printf "\n";
for {t in 1..lm by 3}
{     for {m in t..t+2}
      {  printf "%7s%-14s", "", time2str(str2time(m, "%m"), "%B");
         if m < t+2 then printf "   ";
      }
      printf "\n";
      for {m in t..t+2}
      {  printf "  S  M Tu  W Th  F  S";
         if m < t+2 then printf "   ";
      }
      printf "\n";
      for {k in 0..5}
      {  for {m in t..t+2}
         {  for {d in 0..6}
            {  if cal[m,k,d]  = 0 then printf "   ";
               else printf " %2d", cal[m,k,d];
            }
            if m < t+2 then printf "   ";
         }
         printf "\n";
      }
}
printf "\n";

end;
