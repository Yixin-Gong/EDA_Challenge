#!/bin/python3

import os
import sys
from decimal import Decimal

import xlwt
from tqdm import trange


def set_worktable_style(is_title):
    style = xlwt.XFStyle()
    alignment = xlwt.Alignment()
    alignment.vert = 0x01
    alignment.horz = 0x02
    alignment.wrap = 0
    style.alignment = alignment
    borders = xlwt.Borders()
    borders.left = xlwt.Borders.THIN
    borders.right = xlwt.Borders.THIN
    borders.top = xlwt.Borders.THIN
    borders.bottom = xlwt.Borders.THIN
    style.borders = borders
    if is_title:
        pattern = xlwt.Pattern()
        pattern.pattern = xlwt.Pattern.SOLID_PATTERN
        pattern.pattern_fore_colour = 31
        style.pattern = pattern
    return style


def run_10_times(sheet, name):
    style = set_worktable_style(False)
    with trange(20) as t:
        for round_num in t:
            t.set_description('Running')
            t.set_postfix(file=name, round=round_num)
            call_command = '../build/EDA_Challenge --glitch --file ../testcase/' + name + "/test.vcd"
            out_string = os.popen(call_command).readlines()
            os.remove('./glitch.csv')
            os.remove('./summary.csv')
            get_scope_time = Decimal(out_string[8][16:-2:])
            init_flip_time = Decimal(out_string[9][16:-2:])
            get_flip_time = Decimal(out_string[10][15:-2:])
            print_glitch_time = Decimal(out_string[11][19:-2:])
            print_summary_time = Decimal(out_string[12][20:-2:])
            total_time = Decimal(out_string[13][12:-2:])
            col_content = (get_scope_time, init_flip_time, get_flip_time,
                           print_glitch_time, print_summary_time, total_time)
            for i in range(0, 6):
                sheet.write(round_num + 1, i, col_content[i], style)
            sheet.row(round_num + 1).set_style(xlwt.easyxf('font:height 340'))


def create_sheet(book, sheetname):
    style = set_worktable_style(True)
    sheet = book.add_sheet(sheetname, cell_overwrite_ok=True)
    col_name = ('Get Scope', 'Init Flip', 'Get Flip', 'Print Glitch', 'Print Summary', 'Total Time')
    sheet.row(0).set_style(xlwt.easyxf('font:height 400'))
    for i in range(0, 6):
        sheet.write(0, i, col_name[i], style)
        sheet.col(i).width = 256 * 15
    return sheet


if __name__ == '__main__':
    book = xlwt.Workbook(encoding='utf-8', style_compression=0)

    expect_case = ('case6', 'case8', 'case9', 'BM2')

    for case_name in expect_case:
        sheet = create_sheet(book, case_name)
        run_10_times(sheet, case_name)

    book.save('./' + sys.argv[1] + '.xls')
