import fs from "fs";
import nunjucks from "nunjucks";

import pkg from "../../package.json";

import {
  warn
} from "../utils.mjs";

let ast = null;

const H_TEMPLATE = fs.readFileSync(`${pkg.config.TEMPLATE_DIR}/index-h.njk`, "utf-8");
const CPP_TEMPLATE = fs.readFileSync(`${pkg.config.TEMPLATE_DIR}/index-cpp.njk`, "utf-8");

nunjucks.configure({ autoescape: true });

export default function(astReference, includeMemoryLayouts = false) {
  ast = astReference;
  let {bitmasks} = ast;
  let out = {};
  let vars = {
    bitmasks,
    includeMemoryLayouts
  };
  // h
  {
    let template = H_TEMPLATE;
    let output = nunjucks.renderString(template, vars);
    out.header = output;
  }
  // cpp
  {
    let template = CPP_TEMPLATE;
    let output = nunjucks.renderString(template, vars);
    out.source = output;
  }
  return out;
};
