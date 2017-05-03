'use strict';

String.prototype.padLeft = function (len, c) {
    let _str = this;

    if (_str.length > len) {
        return _str.slice((0 - len));
    } else {
        while (_str.length < len) {
            _str = c + _str;
        }
        return _str;
    }
}

String.format = function () {
    if (arguments.length == 0)
        return null;

    let str = arguments[0];
    for (let i = 1; i < arguments.length; i++) {
        let re = new RegExp('\\{' + (i - 1) + '\\}', 'gm');
        str = str.replace(re, arguments[i]);
    }
    return str;
}

function uuid(len, radix) {
    let chars = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz'.split('');
    let uuid = [],
        i;
    radix = radix || chars.length;

    if (len) {
        // Compact form
        for (i = 0; i < len; i++) uuid[i] = chars[0 | Math.random() * radix];
    } else {
        // rfc4122, version 4 form
        let r;

        // rfc4122 requires these characters
        uuid[8] = uuid[13] = uuid[18] = uuid[23] = '-';
        uuid[14] = '4';

        // Fill in random data. At i==19 set the high bits of clock sequence as
        // per rfc4122, sec. 4.1.5
        for (i = 0; i < 36; i++) {
            if (!uuid[i]) {
                r = 0 | Math.random() * 16;
                uuid[i] = chars[(i == 19) ? (r & 0x3) | 0x8 : r];
            }
        }
    }
    return uuid.join('');
}

const UniquePosition = require('./index');

//21 +  1 + 6
let _marketid = 1;

let _scode1 = "300106";
let _uuid1 = uuid(21, 2);
let _suffix1 = String.format("{0}{1}{2}", _uuid1, _marketid, _scode1).padLeft(28, '0');

let up1 = UniquePosition.InitialPosition(_suffix1);
let cv1 = up1.getCompressValue();
console.log(_suffix1 + " -> " + cv1);

let _scode2 = "300107";
let _uuid2 = uuid(21, 2);
let _suffix2 = String.format("{0}{1}{2}", _uuid2, _marketid, _scode2).padLeft(28, '0');

let up2 = UniquePosition.Before(up1, _suffix2);
let cv2 = up2.getCompressValue();
console.log(_suffix2 + " -> " + cv2);

let _scode3 = "300108";
let _uuid3 = uuid(21, 2);
let _suffix3 = String.format("{0}{1}{2}", _uuid3, _marketid, _scode3).padLeft(28, '0');

let up3 = UniquePosition.After(up1, _suffix3);
let cv3 = up3.getCompressValue();
console.log(_suffix3 + " -> " + cv3);

let _scode4 = "300109";
let _uuid4 = uuid(21, 2);
let _suffix4 = String.format("{0}{1}{2}", _uuid4, _marketid, _scode4).padLeft(28, '0');

let up4 = UniquePosition.Between(up2, up1, _suffix4);
let cv4 = up4.getCompressValue();
console.log(_suffix4 + " -> " + cv4);