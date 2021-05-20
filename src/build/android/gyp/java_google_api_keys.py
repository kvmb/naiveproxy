#!/usr/bin/env python
#
# Copyright 2015 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Generates a Java file with API keys.

import argparse
import os
import string
import sys
import zipfile

from util import build_utils

sys.path.append(
    os.path.abspath(os.path.join(sys.path[0], '../../../google_apis')))
import google_api_keys


PACKAGE = 'org.chromium.chrome'
CLASSNAME = 'GoogleAPIKeys'


def GetScriptName():
  return os.path.relpath(__file__, build_utils.DIR_SOURCE_ROOT)


def GenerateOutput(constant_definitions):
  template = string.Template("""
// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is autogenerated by
//     ${SCRIPT_NAME}
// From
//     ${SOURCE_PATH}

package ${PACKAGE};

public class ${CLASS_NAME} {
${CONSTANT_ENTRIES}
}
""")

  constant_template = string.Template(
      '  public static final String ${NAME} = "${VALUE}";')
  constant_entries_list = []
  for constant_name, constant_value in constant_definitions.iteritems():
    values = {
        'NAME': constant_name,
        'VALUE': constant_value,
    }
    constant_entries_list.append(constant_template.substitute(values))
  constant_entries_string = '\n'.join(constant_entries_list)

  values = {
      'CLASS_NAME': CLASSNAME,
      'CONSTANT_ENTRIES': constant_entries_string,
      'PACKAGE': PACKAGE,
      'SCRIPT_NAME': GetScriptName(),
      'SOURCE_PATH': 'google_api_keys/google_api_keys.h',
  }
  return template.substitute(values)


def _DoWriteJavaOutput(output_path, constant_definition):
  folder = os.path.dirname(output_path)
  if folder and not os.path.exists(folder):
    os.makedirs(folder)
  with open(output_path, 'w') as out_file:
    out_file.write(GenerateOutput(constant_definition))


def _DoWriteJarOutput(output_path, constant_definition):
  folder = os.path.dirname(output_path)
  if folder and not os.path.exists(folder):
    os.makedirs(folder)
  with zipfile.ZipFile(output_path, 'w') as srcjar:
    path = '%s/%s' % (PACKAGE.replace('.', '/'), CLASSNAME + '.java')
    data = GenerateOutput(constant_definition)
    build_utils.AddToZipHermetic(srcjar, path, data=data)


def _DoMain(argv):
  parser = argparse.ArgumentParser()
  parser.add_argument("--out", help="Path for java output.")
  parser.add_argument("--srcjar", help="Path for srcjar output.")
  options = parser.parse_args(argv)
  if not options.out and not options.srcjar:
    parser.print_help()
    sys.exit(-1)

  values = {}
  values['GOOGLE_API_KEY'] = google_api_keys.GetAPIKey()
  values['GOOGLE_API_KEY_PHYSICAL_WEB_TEST'] = (google_api_keys.
      GetAPIKeyPhysicalWebTest())
  values['GOOGLE_CLIENT_ID_MAIN'] = google_api_keys.GetClientID('MAIN')
  values['GOOGLE_CLIENT_SECRET_MAIN'] = google_api_keys.GetClientSecret('MAIN')
  values['GOOGLE_CLIENT_ID_CLOUD_PRINT'] = google_api_keys.GetClientID(
      'CLOUD_PRINT')
  values['GOOGLE_CLIENT_SECRET_CLOUD_PRINT'] = google_api_keys.GetClientSecret(
      'CLOUD_PRINT')
  values['GOOGLE_CLIENT_ID_REMOTING'] = google_api_keys.GetClientID('REMOTING')
  values['GOOGLE_CLIENT_SECRET_REMOTING'] = google_api_keys.GetClientSecret(
      'REMOTING')
  values['GOOGLE_CLIENT_ID_REMOTING_HOST'] = google_api_keys.GetClientID(
      'REMOTING_HOST')
  values['GOOGLE_CLIENT_SECRET_REMOTING_HOST'] = (google_api_keys.
      GetClientSecret('REMOTING_HOST'))
  values['GOOGLE_CLIENT_ID_REMOTING_IDENTITY_API'] = (google_api_keys.
      GetClientID('REMOTING_IDENTITY_API'))

  if options.out:
    _DoWriteJavaOutput(options.out, values)
  if options.srcjar:
    _DoWriteJarOutput(options.srcjar, values)


if __name__ == '__main__':
  _DoMain(sys.argv[1:])
