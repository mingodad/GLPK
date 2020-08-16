#model to test memory usage
param p1;
set s1;
set s2;
set s3;
set s4:=1..p1;
set s5;
set s6;
param p2{s6,s1} default 0;
param p3{s1,s3} default 0;
param p4{s1,s4} default 0;
param p5{s1} default 0;
param p6{s6};

var v1{s1} >=0;
var v2{s6} binary;
var v3{s1} >=0;

maximize omax: sum{a in s6} p6[a]*v2[a]-sum{b in s1} 0.0001*v1[b];
subj to c1{a in s1}: v1[a] +v3[a]= sum{b in s3} p3[a,b]*v1[b]+ sum{c in s6} p2[c,a]*v2[c];
subj to c2{a in s1}: v3[a]<= p5[a]+sum{b in s4} p4[a,b];


