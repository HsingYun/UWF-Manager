#!/usr/bin/env python3
"""Extract Qt widget translations unchanged from a Qt source tree."""
import argparse
import copy
import pathlib
import shutil
import xml.etree.ElementTree as ET

CONTEXTS = {
    'QAbstractSpinBox', 'QLineEdit', 'QWidgetTextControl', 'QPlatformTheme',
    'QDialogButtonBox', 'QFileDialog', 'QFileSystemModel', 'QMessageBox',
    'QTextControl', 'QUnicodeControlCharacterMenu',
}
LOCALES = 'zh_CN zh_TW ja ko de fr es it pt_BR ru uk pl nl tr ar'.split()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('qt_source', type=pathlib.Path)
    args = parser.parse_args()
    source = args.qt_source / 'qttranslations' / 'translations'
    destination = pathlib.Path(__file__).resolve().parent
    destination.mkdir(parents=True, exist_ok=True)
    for locale in LOCALES:
        original = source / f'qtbase_{locale}.ts'
        upstream = ET.parse(original).getroot()
        extracted = ET.Element('TS', upstream.attrib)
        for context in upstream.findall('context'):
            if context.findtext('name') in CONTEXTS:
                extracted.append(copy.deepcopy(context))
        ET.indent(extracted, space='    ')
        text = ET.tostring(extracted, encoding='unicode')
        (destination / original.name).write_text('<?xml version="1.0" encoding="utf-8"?>\n<!DOCTYPE TS>\n' + text + '\n', encoding='utf-8')
        print(f'{locale}: {len(extracted.findall(".//message"))} Qt messages')
    shutil.copyfile(destination / 'qtbase_pt_BR.ts', destination / 'qtbase_pt_PT.ts')


if __name__ == '__main__':
    main()
