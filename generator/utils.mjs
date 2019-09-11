import fs from "fs";

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
