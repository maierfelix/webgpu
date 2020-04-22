import fs from "fs";
import path from "path";

export function warn() {
  let args = [];
  for (let ii = 0; ii < arguments.length; ++ii) args.push(arguments[ii]);
  let str = args.join(", ");
  console.log(`\x1b[33m%s\x1b[0m`, `Warning: ${str}`);
};

export function error() {
  let args = [];
  for (let ii = 0; ii < arguments.length; ++ii) args.push(arguments[ii]);
  let str = args.join(", ");
  process.stderr.write(`\x1b[31mError: ${str}\n\x1b[0m`);
};

export function getPlatform() {
  let fakePlatform = process.env.npm_config_fake_platform;
  if (fakePlatform) {
    switch (fakePlatform) {
      case "win32":
      case "linux":
      case "darwin":
        break;
      default:
        throw new Error(`Invalid fake platform! Aborting..`);
    };
    return fakePlatform;
  }
  return process.platform;
};

export function getCamelizedName(name) {
  return (
    name
    .replace(/\s(.)/g, _ => _.toUpperCase())
    .replace(/\s/g, '')
    .replace(/^(.)/, _ => _.toLowerCase())
  );
};

export function getSnakeCaseName(name) {
  return (
    name
    .replace(/\s/g, "_")
    .toUpperCase()
  );
};

// "R16 float" -> "r16float"
// "depth24 plus stencil8" -> "depth24plus-stencil8"
export function getEnumNameFromDawnEnumName(name) {
  let chunks = name.split(/\s/g);
  chunks = chunks.map((v, i) => {
    if (i < chunks.length - 1) {
      if (v === "1D" || v === "2D" || v === "3D") return v + "-";
      else if (v.match(/^(?=.*[a-zA-Z])(?=.*[0-9])[a-zA-Z0-9]+$/gm)) return v;
      else return v + "-";
    }
    return v;
  });
  chunks = chunks.map(v => v.toLowerCase());
  let out = chunks.join("");
  return out;
};

export function getSnakeCaseFromCamelCaseName(name) {
  return name.split(/(?=[A-Z])/).join('_').toUpperCase();
};

export function firstLetterToUpperCase(str) {
  return str[0].toUpperCase() + str.substr(1);
};

export function isQuotedString(str) {
  return !!((String(str)).match(/"[^"]*"/g));
};

export function normalizeDawnPath(p) {
  p = p.replace(/\s+/g, "");
  if (!path.isAbsolute(p)) {
    throw new Error(`PATH_TO_DAWN must be an absolute path`);
  }
  return p;
};
