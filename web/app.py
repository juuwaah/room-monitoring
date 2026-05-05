import os
from datetime import datetime, timezone
from flask import Flask, request, jsonify, render_template

app = Flask(__name__)

# In-memory status store
current_status = {
    "name": "Staff",
    "status": "unknown",
    "until_time": "",
    "updated_at": None,
}


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/api/status", methods=["GET"])
def get_status():
    return jsonify(current_status)


@app.route("/api/status", methods=["POST"])
def update_status():
    data = request.get_json(force=True)

    status = data.get("status")
    until_time = data.get("until_time", "")

    if status not in ("in_room", "out"):
        return jsonify({"error": "status must be 'in_room' or 'out'"}), 400

    # Validate until_time: must be "0" (indefinite) or 4-digit HHMM
    if until_time == "0":
        pass  # indefinite
    elif len(until_time) == 4 and until_time.isdigit():
        hh = int(until_time[:2])
        mm = int(until_time[2:])
        if hh > 23 or mm > 59:
            return jsonify({"error": "invalid time format"}), 400
    else:
        return jsonify({"error": "until_time must be '0' or 'HHMM'"}), 400

    current_status["status"] = status
    current_status["until_time"] = until_time
    current_status["updated_at"] = datetime.now(timezone.utc).isoformat()

    return jsonify({"ok": True, "status": current_status})


if __name__ == "__main__":
    port = int(os.environ.get("PORT", 5000))
    app.run(host="0.0.0.0", port=port, debug=True)
