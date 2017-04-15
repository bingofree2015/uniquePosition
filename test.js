'use strict';

const UP = require('./build/Release/UniquePosition');
const UniquePosition = UP.UniquePosition;

function UniquePositionInit() {
    var up = new UniquePosition('123');
    up = UniquePosition('123');
}

//UniquePositionInit();

var up1 = UP.InitialPosition("aaaaaaaaaaaaaaaaaaaaaaa99981");
var compressValue1 = up1.getCompressValue();
console.log(compressValue1);

var suffix2 = 'qqqqqqaaaaiiiuuuuuu788909834';
var up2 = UP.Before(up1, suffix2);
var compressValue2 = up2.getCompressValue();
console.log(compressValue2);

var suffix3 = "qqqqqqaaaaiiiuuiiiuu78890987";
var up3 = UP.After(up1, suffix3);
var compressValue3 = up3.getCompressValue();
console.log(compressValue3);

var suffix4 = "qsdfqqqqasfdiiiuuiiiuu788wer";
var up4 = UP.Between(up1, up3, suffix4);
var compressValue4 = up4.getCompressValue();
console.log(compressValue4);

// console.log(UP.IsValidBytes);
// console.log(UP.IsValidSuffix);
// console.log(UP.CreateInvalid);
// console.log(UP.InitialPosition);
// console.log(UP.FromInt64);
// console.log(UP.Before);
// console.log(UP.After);
// console.log(UP.Between);

// test();

function test() {
    console.log(up1.getCompressValue);
    console.log(up1.ToDebugString);

    console.log(up1.Equals);
    console.log(up1.GetSuffixForTest);
    console.log(up1.ToInt64);
    console.log(up1.IsValid);
    console.log(up1.LessThan);
}
