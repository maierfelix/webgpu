import fs from "fs";
import nunjucks from "nunjucks";

import pkg from "../../package.json";

import {
  warn
} from "../utils.mjs";

let ast = null;

const GYP_TEMPLATE = fs.readFileSync(`${pkg.config.TEMPLATE_DIR}/binding-gyp.njk`, "utf-8");

nunjucks.configure({ autoescape: true });

export default function(astReference) {
  ast = astReference;
  let out = {};
  let vars = {
    SOURCE_INCLUDES: [
      "src/index.cpp"
    ].map(v => `"${v}"`)
  };
  // binding.gyp
  {
    let template = GYP_TEMPLATE;
    let output = nunjucks.renderString(template, vars);
    out.gyp = output;
  }
  return out;
};
