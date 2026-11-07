#!/usr/bin/env python3
"""Regression guard: patient identity fields must never be interpolated into logs."""
from pathlib import Path

source = Path("src/core/Application.cpp").read_text(encoding="utf-8")
start = source.index("connect(m_patientTab, &tab::PatientTab::patientAdded")
end = source.index("// Connect PatientTab to MeasurementTab", start)
handler = source[start:end]

assert 'm_logger->info("Patient record created")' in handler
assert 'addDebugMessage("Patient record created", "INFO")' in handler
for sensitive_name in ("pesel", "firstName", "lastName"):
    assert sensitive_name not in handler, (
        f"Patient identity field {sensitive_name} referenced in patient-added handler"
    )
for interpolation in (".arg(", "QString::asprintf", "std::format"):
    assert interpolation not in handler, (
        f"Potential patient-data interpolation remains: {interpolation}"
    )

print("OK: patient identity is not interpolated into application/debug logs")
