import React, { JSX } from "react";
import {
  CssBaseline,
  Container,
  Typography,
  Box,
  Button,
  Grid,
  Paper,
} from "@mui/material";
import { Code, Memory, OpenInNew } from "@mui/icons-material";
import Demo from "./Demos/Demo";
import FileSystemDemo from "./Demos/FileSystemDemo";

const HomeCode = () => (
  <>
    <CssBaseline /> {/* This resets default margins and paddings */}
    <Box
      sx={{
        minHeight: "100vh",
        display: "flex",
        alignItems: "center",
        justifyContent: "center",
        background: "linear-gradient(to bottom right, #1e1e1e, #000000)",
        p: 3,
      }}
    >
      <Container maxWidth="sm">
        <Paper
          elevation={8}
          sx={{ p: 4, borderRadius: 3, backgroundColor: "#2c2c2c" }}
        >
          <Grid
            container
            spacing={3}
            justifyContent="center"
            alignItems="center"
          >
            <Grid item>
              <Box sx={{ bgcolor: "blue", p: 2, borderRadius: "50%" }}>
                <Code sx={{ fontSize: 40, color: "white" }} />
              </Box>
            </Grid>
            <Grid item>
              <Typography variant="h4" color="white">
                +
              </Typography>
            </Grid>
            <Grid item>
              <Box sx={{ bgcolor: "purple", p: 2, borderRadius: "50%" }}>
                <Memory sx={{ fontSize: 40, color: "white" }} />
              </Box>
            </Grid>
          </Grid>

          <Typography
            variant="h4"
            align="center"
            sx={{
              mt: 3,
              background: "linear-gradient(to right, #42a5f5, #7e57c2)",
              WebkitBackgroundClip: "text",
              WebkitTextFillColor: "transparent",
            }}
          >
            React + Membrane
          </Typography>

          <Typography color="gray" align="center" mt={2}>
            A powerful combination for building modern web applications
          </Typography>

          <Box textAlign="center" mt={4}>
          </Box>
        </Paper>
        <Typography textAlign="center" color="gray" mt={3}>
          Building the future of web development
        </Typography>
      </Container>
    </Box>
  </>
);

type pageValue = {
  page: string;
  value: JSX.Element;
}

const pages: pageValue[] = [
  {
    page: "Demo",
    value: <Demo />
  },
  {
    page: "FileSystemDemo",
    value: <FileSystemDemo />,
  },
  {
    page: "HomeCode",
    value: <HomeCode/>,
  },
];

const AppWithSelector = () => {
  const [selectedPage, setSelectedPage] = React.useState("HomeCode");

  const handlePageChange = (pageName: string) => {
    setSelectedPage(pageName);
  };

  const currentPage = pages.find(p => p.page === selectedPage)?.value || pages[2].value;

  return (
    <div>
      <Box sx={{ 
        mb: 2, 
        p: 2, 
        bgcolor: "#2c2c2c", 
        display: "flex", 
        alignItems: "center",
        overflowX: "auto"
      }}>
        <Typography color="white" sx={{ mr: 2, flexShrink: 0 }}>
          Components:
        </Typography>
        <Box sx={{ display: "flex", gap: 1 }}>
          {pages.map((page) => (
            <Button
              key={page.page}
              variant={selectedPage === page.page ? "contained" : "outlined"}
              onClick={() => handlePageChange(page.page)}
              sx={{
                color: "white",
                borderColor: "#555",
                backgroundColor: selectedPage === page.page ? "#444" : "transparent",
                "&:hover": {
                  backgroundColor: selectedPage === page.page ? "#555" : "rgba(255,255,255,0.1)"
                }
              }}
            >
              {page.page}
            </Button>
          ))}
        </Box>
      </Box>
      {currentPage}
    </div>
  );
};

export const App = () => {
  return (
    <div>
      <CssBaseline />
      <AppWithSelector />
    </div>
  );
};
