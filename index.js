const fs = require("fs");
const path = require("path");

const pkg = require("./package.json");

let {platform} = process;
/*
process.stdout.write(`(nvk) Validation checks are enabled\n`);
module.exports = require(`${generatedPath}/interfaces.js`);
*/

const dawnVersion = "0.0.1";

const releasePath = `${pkg.config.GEN_OUT_DIR}/${dawnVersion}/${platform}/build/Release`;
const bindingsPath = path.join(__dirname, `${pkg.config.GEN_OUT_DIR}/`);
const generatedPath = bindingsPath + `${dawnVersion}/${platform}`;

console.log(`${generatedPath}/build/Release/addon-${platform}.node`);
module.exports = require(`${generatedPath}/build/Release/addon-${platform}.node`);
