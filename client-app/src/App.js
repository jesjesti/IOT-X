import React, { useState } from "react";
import "./App.css";
import Box from "@mui/material/Box";
import Paper from "@mui/material/Paper";
import Typography from "@mui/material/Typography";
import Slider from "@mui/material/Slider";
import LinearProgress from "@mui/material/LinearProgress";
import Fab from "@mui/material/Fab";
import Stack from "@mui/material/Stack";
import ArrowUpwardIcon from "@mui/icons-material/ArrowUpward";
import ArrowDownwardIcon from "@mui/icons-material/ArrowDownward";
import ArrowBackIcon from "@mui/icons-material/ArrowBack";
import ArrowForwardIcon from "@mui/icons-material/ArrowForward";

function App() {
  const [servoAngle, setServoAngle] = useState(90);

  const sendCommand = async (operation, state) => {
    try {
      const url = `http://192.168.4.1/${encodeURIComponent(
        operation
      )}?state=${encodeURIComponent(state)}`;
      // fire-and-forget; log result for debug
      const res = await fetch(url, { method: "GET" });
      console.log(`Sent ${operation} ${state} `, res.status);
    } catch (err) {
      console.error("Command send failed", err);
    }
  };

  // new: send servo angle
  const sendServoAngle = async (angle) => {
    try {
      const a = Math.round(Number(angle)); // ensure natural number
      const url = `http://192.168.4.1/rotor/servo?angle=${encodeURIComponent(
        a
      )}`;
      const res = await fetch(url, { method: "GET" });
      console.log(`Sent servo angle=${a}`, res.status);
    } catch (err) {
      console.error("Servo send failed", err);
    }
  };

  // call when press (start)
  const handlePressStart = (operation) => {
    sendCommand(operation, "start");
  };

  // call when release/leave (stop)
  const handlePressStop = (operation) => {
    sendCommand(operation, "stop");
  };

  // helper props for a press-and-hold control (mouse + touch)
  const pressHandlers = (operation) => ({
    onMouseDown: () => handlePressStart(operation),
    onMouseUp: () => handlePressStop(operation),
    onMouseLeave: () => handlePressStop(operation),
    onTouchStart: (e) => {
      e.preventDefault();
      handlePressStart(operation);
    },
    onTouchEnd: () => handlePressStop(operation),
  });

  return (
    <Box
      sx={{
        minHeight: "100vh",
        display: "flex",
        alignItems: "center",
        justifyContent: "center",
        bgcolor: "#f5f7fa",
        p: 2,
      }}
    >
      <Paper elevation={3} sx={{ p: 3, width: 360 }}>
        <Typography variant="h6" align="center" gutterBottom>
          Remote Control
        </Typography>
        <br />
        <Typography variant="h7" align="center" gutterBottom>
          Wheels control
        </Typography>

        <Box sx={{ display: "flex", justifyContent: "center", mb: 2 }}>
          <Stack spacing={1} alignItems="center">
            <Fab
              color="primary"
              aria-label="forward"
              size="medium"
              {...pressHandlers("forward")}
            >
              <ArrowUpwardIcon />
            </Fab>

            <Box sx={{ display: "flex", gap: 2, alignItems: "center" }}>
              <Fab
                color="primary"
                aria-label="rotateLeft"
                size="medium"
                {...pressHandlers("rotateLeft")}
              >
                <ArrowBackIcon />
              </Fab>

              <Fab
                color="secondary"
                aria-label="stop"
                onClick={() => sendCommand("stop", "stop")}
                size="medium"
              >
                <Typography sx={{ fontWeight: 700 }}>●</Typography>
              </Fab>

              <Fab
                color="primary"
                aria-label="rotateRight"
                size="medium"
                {...pressHandlers("rotateRight")}
              >
                <ArrowForwardIcon />
              </Fab>
            </Box>

            <Fab
              color="primary"
              aria-label="backward"
              size="medium"
              {...pressHandlers("backward")}
            >
              <ArrowDownwardIcon />
            </Fab>
          </Stack>
        </Box>

        {/* new: servo control slider + progress */}
        <Box sx={{ mt: 2 }}>
          <Typography gutterBottom>Rotor control</Typography>

          <Slider
            value={servoAngle}
            min={0}
            max={180}
            step={1} // natural numbers only
            onChange={(e, val) => {
              const v = Math.round(Number(val));
              sendServoAngle(v);
            }}
            valueLabelDisplay="auto"
            aria-labelledby="servo-slider"
          />
        </Box>
      </Paper>
    </Box>
  );
}

export default App;
