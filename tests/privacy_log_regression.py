#!/usr/bin/env python3
"""Regression guard: patient identity must not be interpolated into logs."""
from pathlib import Path

source = Path("src/core/Application.cpp").read_text(encoding="utf-8")
start = source.index("connect(m_patientTab, &tab::PatientTab::patientAdded")
end = source.index("connect(m_patientTab, &tab::PatientTab::patientRecordChanged", start)
handler = source[start:end]

assert "Patient chart added [personal fields redacted]" in handler
for sensitive_name in ("pesel", "firstName", "lastName"):
    assert sensitive_name not in handler, (
        f"Patient identity field {sensitive_name} referenced in patient-added handler"
    )
for interpolation in (".arg(", "QString::asprintf", "std::format"):
    assert interpolation not in handler, (
        f"Potential patient-data interpolation remains: {interpolation}"
    )

print("OK: patient identity is excluded from patient-added logs")
