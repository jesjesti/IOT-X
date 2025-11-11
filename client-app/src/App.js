// ...existing code...
import React, { useState } from "react";
import "./App.css";
import Box from "@mui/material/Box";
import Paper from "@mui/material/Paper";
import Typography from "@mui/material/Typography";
import Slider from "@mui/material/Slider";
import Fab from "@mui/material/Fab";
import Stack from "@mui/material/Stack";
import ArrowUpwardIcon from "@mui/icons-material/ArrowUpward";
import ArrowDownwardIcon from "@mui/icons-material/ArrowDownward";
import ArrowBackIcon from "@mui/icons-material/ArrowBack";
import ArrowForwardIcon from "@mui/icons-material/ArrowForward";

function App() {
  const [speed, setSpeed] = useState(50);

  const handleMove = (dir) => {
    // Replace with actual command/send logic (WebSocket/HTTP)
    console.log(`Move ${dir} at speed ${speed}`);
  };

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

        <Box sx={{ display: "flex", justifyContent: "center", mb: 2 }}>
          <Stack spacing={1} alignItems="center">
            <Fab
              color="primary"
              aria-label="forward"
              onClick={() => handleMove("forward")}
              size="medium"
            >
              <ArrowUpwardIcon />
            </Fab>

            <Box sx={{ display: "flex", gap: 2, alignItems: "center" }}>
              <Fab
                color="primary"
                aria-label="left"
                onClick={() => handleMove("left")}
                size="medium"
              >
                <ArrowBackIcon />
              </Fab>

              <Fab
                color="secondary"
                aria-label="stop"
                onClick={() => handleMove("stop")}
                size="medium"
              >
                {/* simple stop is center empty Fab */}
                <Typography sx={{ fontWeight: 700 }}>●</Typography>
              </Fab>

              <Fab
                color="primary"
                aria-label="right"
                onClick={() => handleMove("right")}
                size="medium"
              >
                <ArrowForwardIcon />
              </Fab>
            </Box>

            <Fab
              color="primary"
              aria-label="backward"
              onClick={() => handleMove("backward")}
              size="medium"
            >
              <ArrowDownwardIcon />
            </Fab>
          </Stack>
        </Box>

        <Box sx={{ mt: 1 }}>
          <Typography gutterBottom>Speed: {speed}</Typography>
          <Slider
            value={speed}
            onChange={(e, val) => setSpeed(val)}
            aria-labelledby="speed-slider"
            min={0}
            max={100}
            step={1}
            valueLabelDisplay="auto"
          />
        </Box>
      </Paper>
    </Box>
  );
}

export default App;
// ...existing code...
