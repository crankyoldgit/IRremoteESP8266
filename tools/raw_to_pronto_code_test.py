#!/usr/bin/python3
"""Unit tests for raw_to_pronto_code.py"""
from io import StringIO
import unittest
import raw_to_pronto_code as pronto

class TestRawToPronto(unittest.TestCase):
  """Unit tests for the methods in raw_to_pronto_code."""

  def test_parse_and_report_at_38000(self):
    """Tests for the parse_and_report() function @ 38kHz."""

    output = StringIO()
    input_str = """
        uint16_t rawData[7] = {
            20100, 20472, 15092, 30704, 20102, 20472, 15086};"""
    pronto.parse_and_report(input_str, 38000, 100000, False, output)
    self.assertEqual(
        output.getvalue(),
        "Pronto code = "
        "'0000 006D 0004 0000 02FB 0309 023D 048E 02FB 0309 023D 0ED8'\n")

  def test_parse_and_report_at_36000(self):
    """Tests for the parse_and_report() function @ 36kHz."""

    output = StringIO()
    input_str = """
        uint16_t rawData[7] = {
            20100, 20472, 15092, 30704, 20102, 20472, 15086};"""
    pronto.parse_and_report(input_str, 36000, 100000, False, output)
    self.assertEqual(
        output.getvalue(),
        "Pronto code = "
        "'0000 0073 0004 0000 02D3 02E0 021F 0451 02D3 02E0 021F 0E10'\n")

  def test_parse_and_report_at_57600(self):
    """Tests for the parse_and_report() function @ 57.6kHz."""

    output = StringIO()
    input_str = """
      uint16_t rawData[7] = {
          20100, 20472, 15092, 30704, 20102, 20472, 15086};"""
    pronto.parse_and_report(input_str, 57600, 100000, False, output)
    self.assertEqual(
        output.getvalue(),
        "Pronto code = "
        "'0000 0047 0004 0000 0485 049B 0365 06E8 0485 049B 0364 1680'\n")


if __name__ == '__main__':
  unittest.main(verbosity=2)
