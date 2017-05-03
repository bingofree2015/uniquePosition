'use strict';

const up = require('./index');
//const up = require('./build/Release/UniquePosition');

let suffix1 = "01|02|300106|00bingofree2015";
let up1 = up.InitialPosition(suffix1);
let cv1 = up1.getCompressValue(true);
let cvi1 = up1.ToInt64();
console.log(suffix1 + " -> " + cv1 + " -> " + cvi1);

let suffix2 = '02|01|3000208|bingo789free78';
let up2 = up.Before(up1, suffix2);
let cv2 = up2.getCompressValue(true);
let cvi2 = up2.ToInt64();
console.log(suffix2 + " -> " + cv2 + " -> " + cvi2);

let suffix3 = "03|04|100789|00000kvkv009876";
let up3 = up.After(up1, suffix3);
let cv3 = up3.getCompressValue(true);
let cvi3 = up3.ToInt64();
console.log(suffix3 + " -> " + cv3 + " -> " + cvi3);

let suffix4 = "04|02|300986|00000alix98yoll";
let up4 = up.Between(up1, up3, suffix4);
let cv4 = up4.getCompressValue(true);
let cvi4 = up4.ToInt64();
console.log(suffix4 + " -> " + cv4 + " -> " + cvi4);

