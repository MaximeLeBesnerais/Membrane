import React from 'react';
import { CssBaseline, Container, Typography, Box, Button, Grid, Paper } from '@mui/material';
import { Code, Memory, OpenInNew } from '@mui/icons-material';

export const App = () => {
  return (
    <>
      <CssBaseline /> {/* This resets default margins and paddings */}
      <Box
        sx={{
          minHeight: '100vh',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          background: 'linear-gradient(to bottom right, #1e1e1e, #000000)',
          p: 3,
        }}
      >
        <Container maxWidth="sm">
          <Paper elevation={8} sx={{ p: 4, borderRadius: 3, backgroundColor: '#2c2c2c' }}>
            <Grid container spacing={3} justifyContent="center" alignItems="center">
              <Grid item>
                <Box sx={{ bgcolor: 'blue', p: 2, borderRadius: '50%' }}>
                  <Code sx={{ fontSize: 40, color: 'white' }} />
                </Box>
              </Grid>
              <Grid item>
                <Typography variant="h4" color="white">+</Typography>
              </Grid>
              <Grid item>
                <Box sx={{ bgcolor: 'purple', p: 2, borderRadius: '50%' }}>
                  <Memory sx={{ fontSize: 40, color: 'white' }} />
                </Box>
              </Grid>
            </Grid>

            <Typography
              variant="h4"
              align="center"
              sx={{
                mt: 3,
                background: 'linear-gradient(to right, #42a5f5, #7e57c2)',
                WebkitBackgroundClip: 'text',
                WebkitTextFillColor: 'transparent',
              }}
            >
              React + Membrane
            </Typography>

            <Typography color="gray" align="center" mt={2}>
              A powerful combination for building modern web applications
            </Typography>

            <Box textAlign="center" mt={4}>
              <Button
                variant="contained"
                color="primary"
                endIcon={<OpenInNew />}
                sx={{
                  background: 'linear-gradient(to right, #42a5f5, #7e57c2)',
                  color: 'white',
                  '&:hover': { opacity: 0.9 },
                }}
              >
                Get Started
              </Button>
            </Box>
          </Paper>
          <Typography textAlign="center" color="gray" mt={3}>
            Building the future of web development
          </Typography>
        </Container>
      </Box>
    </>
  );
};
